#include "HttpRequest.h"

#include <cctype>
#include <cstddef>
#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Http.h"
#include "Utils.h"

#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#  include <algorithm>
#endif

HttpRequest::MethodMap initMethodMap() {
  HttpRequest::MethodMap map;
  map.insert(std::pair<std::string, enum Http::method>("GET", Http::GET));
  map.insert(std::pair<std::string, enum Http::method>("POST", Http::POST));
  map.insert(std::pair<std::string, enum Http::method>("DELETE", Http::DELETE));
  return map;
}
const HttpRequest::MethodMap HttpRequest::method_map = initMethodMap();

// NOLINTNEXTLINE
HttpRequest::HttpRequest(std::string const& raw) : status_(S_NONE), method_(Http::UNKNOWN) {
  enum parse_state state = STATE_METHOD;
  std::string headerName;
  for (std::string::const_iterator it = raw.begin(); it != raw.end() && status_ != S_BAD_REQUEST
                                                     && status_ != S_HTTP_VERSION_NOT_SUPPORTED
                                                     && status_ != S_NOT_IMPLEMENTED;) {
    switch (state) {
      case STATE_METHOD: {
        try {
          std::string method = Utils::getNextToken(raw, it, SP);
          std::map<std::string, enum Http::method>::const_iterator m = method_map.find(method);
          if (m != method_map.end()) {
            method_ = m->second;
            it += method.length();  // NOLINT
            if (*it == SP) {
              ++it;
              state = STATE_URI;
            } else {
              status_ = S_BAD_REQUEST;
            }
          } else {
            status_ = S_NOT_IMPLEMENTED;
          }
        } catch (std::out_of_range) {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      // TODO: validate URI
      case STATE_URI: {
        try {
          std::string uri = Utils::getNextToken(raw, it, SP);
          if (!uri.empty()) {
            // std::transform(uri.begin(), uri.end(), uri.begin(), ::tolower);
            uri_ = uri;
            it += uri.length();  // NOLINT
            if (*it == SP) {
              ++it;
              state = STATE_VERSION;
            } else {
              status_ = S_BAD_REQUEST;
            }
          } else {
            status_ = S_BAD_REQUEST;
          }
        } catch (std::out_of_range) {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case STATE_VERSION: {
        try {
          std::string version = Utils::getNextToken(raw, it, CR);
          if (!version.empty()) {
            try {
              std::string protocol = Utils::getNextToken(version, version.begin(), '/');
              std::string sep1 = version.substr(protocol.length(), 1);
              std::string major = Utils::getNextToken(version, version.begin() + protocol.length() + 1, '.');  // NOLINT
              std::string sep2 = version.substr(protocol.length() + 1 + major.length(), 1);
              std::string minor = version.substr(protocol.length() + 1 + major.length() + 1);
              if (protocol == "HTTP" && sep1 == "/" && major == "1" && sep2 == "." && minor == "1") {
                version_ = version;
                it += version.length();  // NOLINT
                if (*it == CR && *(it + 1) == LF) {
                  it += 2;
                  state = STATE_CRLF;
                } else {
                  status_ = S_BAD_REQUEST;
                }
              } else {
                status_ = S_HTTP_VERSION_NOT_SUPPORTED;
              }
            } catch (std::out_of_range) {
              status_ = S_HTTP_VERSION_NOT_SUPPORTED;
            }
          } else {
            status_ = S_BAD_REQUEST;
          }
        } catch (std::out_of_range) {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case STATE_NAME: {
        try {
          std::string name = Utils::getNextToken(raw, it, ':');
          if (!name.empty() && name.find('(') == std::string::npos && name.find(')') == std::string::npos
              && name.find('<') == std::string::npos && name.find('>') == std::string::npos
              && name.find('@') == std::string::npos && name.find(',') == std::string::npos
              && name.find(';') == std::string::npos && name.find(':') == std::string::npos
              && name.find('\\') == std::string::npos && name.find('"') == std::string::npos
              && name.find('/') == std::string::npos && name.find('[') == std::string::npos
              && name.find(']') == std::string::npos && name.find('?') == std::string::npos
              && name.find('=') == std::string::npos && name.find('{') == std::string::npos
              && name.find('}') == std::string::npos && name.find(SP) == std::string::npos
              && name.find(HT) == std::string::npos && name.find(CR) == std::string::npos
              && name.find(LF) == std::string::npos) {
            std::transform(name.begin(), name.end(), name.begin(), ::tolower);
            headerName = name;
            it += name.length();  // NOLINT
            ++it;
            state = STATE_COLON;
          } else {
            status_ = S_BAD_REQUEST;
          }
        } catch (std::out_of_range) {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case STATE_COLON: {
        if (*it == SP || *it == HT) {
          ++it;
        } else {
          state = STATE_VALUE;
        }
        break;
      }
      case STATE_VALUE: {
        try {
          std::string value = Utils::getNextToken(raw, it, CR);
          if (!headerName.empty()) {
            headers_.insert(std::pair<std::string, std::string>(headerName, value));
            if (headerName == "expect" && value == "100-continue") {
              status_ = S_CONTINUE;
            }
            headerName.clear();
            it += value.length();  // NOLINT
            if (*it == CR && *(it + 1) == LF) {
              it += 2;
              state = STATE_CRLF;
            } else {
              status_ = S_BAD_REQUEST;
            }
          } else {
            status_ = S_BAD_REQUEST;
          }
        } catch (std::out_of_range) {
          status_ = S_BAD_REQUEST;
        }
        break;
      }
      case STATE_CRLF: {
        if (*it == CR && *(it + 1) == LF) {
          if (headers_.find("host") == headers_.end()) {
            status_ = S_BAD_REQUEST;
          } else {
            std::size_t start = std::distance(raw.begin(), it + 2);
            std::size_t end = std::distance(it + 2, raw.end());
            body_ = raw.substr(start, end);
            if (status_ != S_CONTINUE) {
              status_ = S_OK;
            }
          }
        } else if (it == raw.end()) {
          status_ = S_BAD_REQUEST;
        } else {
          state = STATE_NAME;
        }
        break;
      }
    }
  }
}

// TODO
bool HttpRequest::addChunk(std::string const& chunk) {
  if (status_ == S_CONTINUE) {
    body_.append(chunk);
    return true;
  }
  return false;
}

HttpRequest::~HttpRequest() {}

enum HttpRequest::status HttpRequest::getStatus() const { return status_; }

enum Http::method HttpRequest::getMethod() const { return method_; }

std::string HttpRequest::getUri() const { return uri_; }

std::string HttpRequest::getVersion() const { return version_; }

std::map<std::string, std::string> HttpRequest::getHeaders() const { return headers_; }

std::string HttpRequest::getBody() const { return body_; }

std::pair<std::string, std::string> HttpRequest::getHost() {
  std::pair<std::string, std::string> result;
  std::map<std::string, std::string>::iterator host = headers_.find("host");
  if (host != headers_.end()) {
    std::string::size_type sep = host->second.find(':');
    if (sep == std::string::npos) {
      result.first = host->second;
      result.second = "80";
    } else {
      result.first = host->second.substr(0, sep);
      result.second = host->second.substr(sep + 1);
    }
  }
  return result;
}