#include "HttpRequest.h"

#include <cctype>
#include <cstddef>
#include <exception>
#include <string>
#include <utility>

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

// TODO: check request size
// NOLINTNEXTLINE
HttpRequest::HttpRequest(std::string const& raw) : status_(S_NONE), time_(std::time(nullptr)), method_(Http::UNKNOWN) {
  enum req_parse_state state = S_REQ_METHOD;
  std::string header_name;
  std::string::const_iterator last_token;
  for (std::string::const_iterator it = raw.begin(); it != raw.end() && (status_ == S_NONE || status_ == S_CONTINUE);) {
    switch (state) {
      case S_REQ_METHOD: {
        std::string::size_type len = raw.find(SP);
        if (len != std::string::npos) {
          std::string method = raw.substr(0, len);
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
          } else {
            status_ = S_NOT_IMPLEMENTED;
          }
        } else {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case S_REQ_URI: {
        if (*it == SP) {
          std::string uri = raw.substr(std::distance(raw.begin(), last_token), std::distance(last_token, it));
          try {
            uri_ = Uri(uri);
            if (uri_.getType() == Uri::TYPE_RELATIVE) {
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
          std::string version = raw.substr(std::distance(raw.begin(), last_token), std::distance(last_token, it));
          if (version == "HTTP/1.1") {
            if (*it == CR && *(it + 1) == LF) {
              it += 2;
              last_token = it;
              state = S_REQ_CRLF;
            } else {
              status_ = S_BAD_REQUEST;
            }
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
          std::string name = raw.substr(std::distance(raw.begin(), last_token), std::distance(last_token, it));
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
          std::string value = raw.substr(std::distance(raw.begin(), last_token), std::distance(last_token, it));
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
            std::size_t start = std::distance(raw.begin(), it + 2);
            std::size_t end = std::distance(it + 2, raw.end());
            body_ = raw.substr(start, end);
            if (status_ != S_CONTINUE) {
              status_ = S_OK;
            }
          } else {
            status_ = S_BAD_REQUEST;
          }
        } else if (it == raw.end()) {
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
bool HttpRequest::addChunk(std::string const& chunk) {
  (void)chunk;
  if (status_ == S_CONTINUE) {
    // enum chunk_parse_state state = S_CHK_IDENTIFY;
    // for (std::string::const_iterator it = chunk.begin(); it != chunk.end();) {
    //   switch (state) {
    //     case S_CHK_IDENTIFY: {
    //       // chunk: HEX[;*[=*]]CRLF
    //       // last: 0[;*[=*]]CRLF
    //       // trailer: field-name ":" OWS field-value OWS CRLF
    //     }
    //     case S_CHK_SIZE: {
    //     }
    //     case S_CHK_EXT: {
    //     }
    //     case S_CHK_DATA: {
    //     }
    //     case S_CHK_LAST: {
    //     }
    //     case S_CHK_TRAILER: {
    //     }
    //     case S_CHK_CRLF: {
    //     }
    //   }
    // }
    return true;
  }
  return false;
}

HttpRequest::~HttpRequest() {}

enum HttpRequest::status HttpRequest::getStatus() const { return status_; }

std::time_t const& HttpRequest::getTime() const { return time_; }

enum Http::method HttpRequest::getMethod() const { return method_; }

Uri const& HttpRequest::getUri() const { return uri_; }

std::map<std::string, std::string> const& HttpRequest::getHeaders() const { return headers_; }

std::string const& HttpRequest::getBody() const { return body_; }

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
