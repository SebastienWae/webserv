#include "HttpRequest.h"

#include <_types/_uint8_t.h>

#include <__nullptr>
#include <algorithm>
#include <cctype>
#include <cstddef>
#include <exception>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "Config.h"
#include "Http.h"
#include "ServerConfig.h"
#include "Uri.h"

HttpRequest::MethodMap initMethodMap() {
  HttpRequest::MethodMap map;
  map.insert(std::pair<std::string, enum Http::method>("GET", Http::GET));
  map.insert(std::pair<std::string, enum Http::method>("HEAD", Http::HEAD));
  map.insert(std::pair<std::string, enum Http::method>("POST", Http::POST));
  map.insert(std::pair<std::string, enum Http::method>("DELETE", Http::DELETE));
  return map;
}
const HttpRequest::MethodMap HttpRequest::method_map = initMethodMap();

// NOLINTNEXTLINE
HttpRequest::HttpRequest(std::vector<uint8_t> const& data, Config const& config)
    : status_(S_NONE),
      time_(std::time(nullptr)),
      method_(Http::UNKNOWN),
      uri_(NULL),
      content_length_(0),
      is_chunked_(false) {
  enum req_parse_state state = S_REQ_METHOD;
  std::string header_name;
  std::vector<uint8_t>::const_iterator last_token;
  for (std::vector<uint8_t>::const_iterator it = data.begin();
       it != data.end() && (status_ == S_NONE || status_ == S_CONTINUE);) {
    switch (state) {
      case S_REQ_METHOD: {
        std::vector<uint8_t>::const_iterator sep = std::find(it, data.end(), SP);
        if (sep != data.end()) {
          std::string method(it, sep);
          std::map<std::string, enum Http::method>::const_iterator m_it = method_map.find(method);
          if (m_it != method_map.end()) {
            method_ = m_it->second;
            it += method.length();  // NOLINT
            if (*it == SP) {
              ++it;
              last_token = it;
              state = S_REQ_URI;
            } else {
              status_ = S_BAD_REQUEST;
            }
          } else if (method == "OPTIONS" || method == "PUT" || method == "TRACE" || method == "CONNECT") {
            status_ = S_NOT_IMPLEMENTED;
          } else {
            status_ = S_BAD_REQUEST;
          }
        } else {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case S_REQ_URI: {
        if (*it == SP) {
          std::string uri(last_token, it);
          try {
            uri_ = new Uri(uri);
            if (uri_->getType() == Uri::TYPE_RELATIVE) {
              ++it;
              last_token = it;
              state = S_REQ_VERSION;
            } else {
              status_ = S_BAD_REQUEST;
            }
          } catch (Uri::UriParsingException& e) {
            status_ = S_BAD_REQUEST;
          }
        } else if (std::isprint(*it) != 0) {
          ++it;
        } else {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case S_REQ_VERSION: {
        if (*it == CR) {
          std::string version(last_token, it);
          if (version == "HTTP/1.1") {
            if (*it == CR && *(it + 1) == LF) {
              it += 2;
              last_token = it;
              state = S_REQ_CRLF;
            } else {
              status_ = S_BAD_REQUEST;
            }
          } else if (version.empty()) {
            status_ = S_BAD_REQUEST;
          } else {
            status_ = S_HTTP_VERSION_NOT_SUPPORTED;
          }
        } else if (std::isprint(*it) != 0) {
          ++it;
        } else {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case S_REQ_HEADER_NAME: {
        if (*it == ':') {
          std::string name(last_token, it);
          std::transform(name.begin(), name.end(), name.begin(), ::tolower);
          header_name = name;
          ++it;
          last_token = it;
          state = S_REQ_HEADER_SEP;
        } else if (*it != '(' && *it != ')' && *it != '<' && *it != '>' && *it != '@' && *it != ',' && *it != ';'
                   && *it != ':' && *it != '\\' && *it != '"' && *it != '/' && *it != '[' && *it != ']' && *it != '?'
                   && *it != '=' && *it != '{' && *it != '}' && *it != SP && *it != HT && *it != CR && *it != LF) {
          ++it;
        } else {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case S_REQ_HEADER_SEP: {
        if (*it == SP || *it == HT) {
          ++it;
        } else {
          last_token = it;
          state = S_REQ_HEADER_VAL;
        }
        break;
      }
      case S_REQ_HEADER_VAL: {
        if (*it == CR) {
          std::string value(last_token, it);
          if (header_name.empty()) {
            status_ = S_BAD_REQUEST;
          } else {
            headers_[header_name] = value;
            if (header_name == "expect") {
              std::transform(value.begin(), value.end(), value.begin(), ::tolower);
              if (value == "100-continue" && method_ == Http::POST) {
                status_ = S_CONTINUE;
              } else {
                status_ = S_EXPECTATION_FAILED;
              }
            }
            header_name.clear();
            if (status_ == S_NONE || status_ == S_CONTINUE) {
              if (*it == CR && *(it + 1) == LF) {
                it += 2;
                last_token = it;
                state = S_REQ_CRLF;
              } else {
                status_ = S_BAD_REQUEST;
              }
            }
          }
        } else {
          ++it;
        }
        break;
      }
      case S_REQ_CRLF: {
        if (*it == CR && *(it + 1) == LF) {
          std::map<std::string, std::string>::const_iterator h_it = headers_.find("host");
          if (h_it != headers_.end() && !h_it->second.empty()) {
            h_it = headers_.find("transfer-encoding");
            if (h_it != headers_.end() && !h_it->second.empty()) {
              if (h_it->second == "chunked") {
                is_chunked_ = true;
              } else {
                status_ = S_NOT_IMPLEMENTED;
                break;
              }
            }

            h_it = headers_.find("content-length");
            if (h_it != headers_.end() && !h_it->second.empty()) {
              content_length_ = std::atol(h_it->second.c_str());
            } else if (method_ == Http::POST && !is_chunked_) {
              status_ = S_LENGTH_REQUIRED;
              break;
            }

            if (method_ == Http::POST) {
              body_.assign(it + 2, data.end());
              ServerConfig const* sc = config.matchServerConfig(getHost());
              if (content_length_ > static_cast<long>(1 * GIGA) || content_length_ > sc->getMaxBodySize()) {
                status_ = S_REQUEST_ENTITY_TOO_LARGE;
              } else if (body_.size() > content_length_) {
                status_ = S_BAD_REQUEST;
              }
            }

            if (status_ != S_CONTINUE) {
              status_ = S_OK;
            }
          } else {
            status_ = S_BAD_REQUEST;
          }
          it += 2;
        } else if (it == data.end()) {
          status_ = S_BAD_REQUEST;
        } else {
          state = S_REQ_HEADER_NAME;
        }
        break;
      }
    }
  }
}

// TODO: 415 Unsupported Media Type

void HttpRequest::addChunk(std::vector<uint8_t> const& chunk, std::size_t max_body_size) {
  if (is_chunked_) {
    chunks_buff_.insert(chunks_buff_.end(), chunk.begin(), chunk.end());
    parseChunks(max_body_size);
  } else {
    std::size_t total = body_.size() + chunk.size();
    if (total > content_length_) {
      status_ = S_BAD_REQUEST;
    } else {
      if (total == content_length_) {
        status_ = S_OK;
      }
      body_.insert(body_.end(), chunk.begin(), chunk.end());
    }
  }
}

void HttpRequest::parseChunks(std::size_t max_body_size) {
  std::vector<uint8_t>::iterator sep = std::find(chunks_buff_.begin(), chunks_buff_.end(), CR);
  if (sep == chunks_buff_.end() || sep[1] != LF) {
    return;
  }

  std::string hex(chunks_buff_.begin(), sep);
  long size = strtol(hex.c_str(), nullptr, HEX_BASE);

  std::size_t total = body_.size() + chunks_buff_.size();
  if (total > static_cast<long>(1 * GIGA) || total > max_body_size) {
    status_ = S_REQUEST_ENTITY_TOO_LARGE;
    return;
  }

  std::vector<uint8_t>::const_iterator begin = sep + 2;
  std::vector<uint8_t>::const_iterator end = chunks_buff_.end();
  for (std::vector<uint8_t>::const_iterator it = begin; it != end; ++it) {
    if (*it == CR && it[1] == LF) {
      long len = std::distance(begin, it);
      if (size == 0 && len == 0) {
        status_ = S_OK;
        break;
      }
      if (size == len) {
        body_.insert(body_.end(), begin, it);
        chunks_buff_.erase(chunks_buff_.begin(), it + 2);
        parseChunks(max_body_size);
        break;
      }
    }
  }
}

HttpRequest::~HttpRequest() {
  if (uri_ != NULL) {
    delete uri_;
  }
}

enum HttpRequest::status HttpRequest::getStatus() const { return status_; }

std::time_t const& HttpRequest::getTime() const { return time_; }

enum Http::method HttpRequest::getMethod() const { return method_; }

Uri const* HttpRequest::getUri() const { return uri_; }

std::map<std::string, std::string> const& HttpRequest::getHeaders() const { return headers_; }

std::vector<uint8_t> const& HttpRequest::getBody() const { return body_; }

std::string HttpRequest::getHost() const {
  std::map<std::string, std::string>::const_iterator h_it = headers_.find("host");

  if (h_it != headers_.end()) {
    std::string host = h_it->second;

    if (host.find(':') == std::string::npos) {
      host += ":80";
    }
    return host;
  }
  return "";
}

// TODO
bool HttpRequest::isFileUpload() const { return true; }
