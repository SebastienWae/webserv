#include "HttpRequest.h"

#include <_types/_uint8_t.h>

#include <__nullptr>
#include <cctype>
#include <cstddef>
#include <exception>
#include <string>
#include <utility>
#include <vector>

#include "Http.h"
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
HttpRequest::HttpRequest(std::vector<uint8_t> const& data)
    : status_(S_NONE), time_(std::time(nullptr)), method_(Http::UNKNOWN), uri_(NULL) {
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
              if (value == "100-continue") {
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
            body_.assign(it + 2, data.end());
            if (status_ != S_CONTINUE) {
              status_ = S_OK;
            }
            it += 2;
          } else {
            status_ = S_BAD_REQUEST;
          }
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

// TODO
// if host -> bad request
// handle chunk AND content-length
// 411 Length Required
// 415 Unsupported Media Type
// TODO: check request size
bool HttpRequest::addChunk(std::vector<uint8_t> const& chunk) {
  body_.insert(body_.end(), chunk.begin(), chunk.end());
  if (*(chunk.end() - 4) == CR && *(chunk.end() - 3) == LF && *(chunk.end() - 2) == CR && *(chunk.end() - 1) == LF) {
    status_ = S_OK;
  }
  return true;
  // if (status_ == S_CONTINUE) {
  //   // enum chunk_parse_state state = S_CHK_IDENTIFY;
  //   // for (std::string::const_iterator it = chunk.begin(); it != chunk.end();) {
  //   //   switch (state) {
  //   //     case S_CHK_IDENTIFY: {
  //   //       // chunk: HEX[;*[=*]]CRLF
  //   //       // last: 0[;*[=*]]CRLF
  //   //       // trailer: field-name ":" OWS field-value OWS CRLF
  //   //     }
  //   //     case S_CHK_SIZE: {
  //   //     }
  //   //     case S_CHK_EXT: {
  //   //     }
  //   //     case S_CHK_DATA: {
  //   //     }
  //   //     case S_CHK_LAST: {
  //   //     }
  //   //     case S_CHK_TRAILER: {
  //   //     }
  //   //     case S_CHK_CRLF: {
  //   //     }
  //   //   }
  //   // }
  //   return true;
  // }
  // return false;
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
