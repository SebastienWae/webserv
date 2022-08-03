#include "Uri.h"

#include <algorithm>
#include <cctype>
#include <exception>
#include <iterator>
#include <stdexcept>
#include <string>

// NOLINTNEXTLINE
Uri::Uri(std::string const& uri) throw(UriParsingException) : type_(Uri::TYPE_NONE), raw_(uri) {
  if (uri.empty()) {
    throw UriParsingException();
  }
  enum parse_state state = STATE_IDENTIFY;
  std::string::const_iterator last_token;
  try {
    for (std::string::const_iterator it = uri.begin(); it != uri.end();) {
      switch (state) {
        case STATE_IDENTIFY: {
          if (uri.length() > 2 && uri.substr(0, 2) == "//") {
            type_ = TYPE_ABSOLUTE;
            state = STATE_HOST;
            scheme_ = "http";
            it += 2;
            last_token = it;
          } else if (uri.length() >= 1 && uri[0] == '/' && uri[1] != '/') {
            type_ = TYPE_RELATIVE;
            state = STATE_PATH;
            ++it;
            last_token = it;
          } else if (std::isalpha(uri.at(0)) != 0) {
            state = STATE_SCHEME;
            last_token = it;
          } else {
            throw UriParsingException();
          }
          break;
        }
        case STATE_SCHEME: {
          if (std::isalnum(*it) != 0 || *it == '+' || *it == '-' || *it == '+') {
            ++it;
          } else if (*it == ':') {
            scheme_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
            std::transform(scheme_.begin(), scheme_.end(), scheme_.begin(), ::tolower);
            if (scheme_ != "http") {
              throw UriParsingException();
            }
            type_ = TYPE_ABSOLUTE;
            ++it;
            last_token = it;
            if (uri.substr(std::distance(uri.begin(), last_token), 2) == "//") {
              it += 2;
              last_token = it;
              state = STATE_HOST;
            } else {
              throw UriParsingException();
            }
          } else {
            throw UriParsingException();
          }
          break;
        }
        case STATE_HOST: {
          std::size_t dist = std::distance(uri.begin(), it);
          std::string::size_type at = uri.find('@', dist);
          std::string::size_type slash = uri.find('/', dist);
          if (at != std::string::npos && (slash == std::string::npos || at < slash)) {
            state = STATE_USERINFO;
          } else {
            state = STATE_HOST_TYPE;
          }
          break;
        }
        case STATE_HOST_TYPE: {
          std::size_t dist = std::distance(uri.begin(), last_token);
          std::string::size_type slash = uri.find('/', dist);
          std::string host = uri.substr(dist, slash);
          std::string::size_type dot = host.find_last_of('.');
          if (dot == std::string::npos || (std::isalpha(host.at(dot + 1)) != 0)) {
            state = STATE_HOSTNAME;
          } else {
            state = STATE_IPV4ADDRESS;
          }
          break;
        }
        case STATE_USERINFO: {
          if (std::isalnum(*it) != 0 || *it == ';' || *it == ':' || *it == '&' || *it == '=' || *it == '+' || *it == '$'
              || *it == ',' || *it == '-' || *it == '_' || *it == '.' || *it == '!' || *it == '~' || *it == '*'
              || *it == '\'' || *it == '(' || *it == ')' || *it == '%') {
            ++it;
          } else if (*it == '@') {
            userinfo_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
            ++it;
            last_token = it;
            state = STATE_HOST_TYPE;
          } else {
            throw UriParsingException();
          }
          break;
        }
        case STATE_HOSTNAME: {
          if (std::isalnum(*it) != 0 || *it == '.' || *it == '-' || *it == '!' || *it == '$' || *it == '&'
              || *it == '\'' || *it == '(' || *it == ')' || *it == '*' || *it == '+' || *it == ',' || *it == ';'
              || *it == '=') {
            ++it;
            if (it == uri.end() || *it == '/' || *it == ':' || *it == '?' || *it == '#') {
              host_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
              std::transform(host_.begin(), host_.end(), host_.begin(), ::tolower);
              if (*it == '/' || *it == ':' || *it == '?' || *it == '#') {
                if (*it == '/' && *(it + 1) != '/') {
                  state = STATE_PATH;
                } else if (*it == ':') {
                  state = STATE_PORT;
                } else if (*it == '?') {
                  state = STATE_QUERY;
                } else if (*it == '#') {
                  state = STATE_FRAGMENT;
                }
                ++it;
                last_token = it;
              }
            }
          } else {
            throw UriParsingException();
          }
          break;
        }
        case STATE_IPV4ADDRESS: {
          if (std::isdigit(*it) != 0 || *it == '.') {
            ++it;
            if (it == uri.end() || *it == '/' || *it == ':' || *it == '?' || *it == '#') {
              host_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
              if (*it == '/' || *it == ':' || *it == '?' || *it == '#') {
                if (*it == '/' && *(it + 1) != '/') {
                  state = STATE_PATH;
                } else if (*it == ':') {
                  state = STATE_PORT;
                } else if (*it == '?') {
                  state = STATE_QUERY;
                } else if (*it == '#') {
                  state = STATE_FRAGMENT;
                }
                ++it;
                last_token = it;
              }
            }
          } else {
            throw UriParsingException();
          }
          break;
        }
        case STATE_PORT: {
          if (std::isdigit(*it) != 0) {
            ++it;
            if (it == uri.end() || *it == '/' || *it == '?' || *it == '#') {
              port_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
              if (*it == '/' || *it == '#') {
                if (*it == '/' && *(it + 1) != '/') {
                  state = STATE_PATH;
                } else if (*it == '?') {
                  state = STATE_QUERY;
                } else if (*it == '#') {
                  state = STATE_FRAGMENT;
                }
                ++it;
                last_token = it;
              }
            }
          } else {
            throw UriParsingException();
          }
          break;
        }
        case STATE_PATH: {
          if (std::isalnum(*it) != 0 || *it == '\'' || *it == ';' || *it == '=' || *it == '.' || *it == ':'
              || *it == '@' || *it == '&' || *it == '=' || *it == '+' || *it == '$' || *it == ',' || *it == '%'
              || *it == '-' || *it == '_' || *it == '!' || *it == '~' || *it == '*' || *it == '\'' || *it == '('
              || *it == ')' || *it == '/') {
            ++it;
            if (it == uri.end() || *it == '?' || *it == '#') {
              path_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
              if (*it == '?' || *it == '#') {
                if (*it == '?') {
                  state = STATE_QUERY;
                } else if (*it == '#') {
                  state = STATE_FRAGMENT;
                }
                ++it;
                last_token = it;
              }
            }
          } else if (*it == '?' || *it == '#') {
            if (*it == '?') {
              state = STATE_QUERY;
            } else if (*it == '#') {
              state = STATE_FRAGMENT;
            }
            ++it;
            last_token = it;
          } else {
            throw UriParsingException();
          }
          break;
        }
        case STATE_QUERY: {
          if (std::isalnum(*it) != 0 || *it == '.' || *it == ';' || *it == '/' || *it == '?' || *it == ':' || *it == '@'
              || *it == '&' || *it == '=' || *it == '+' || *it == '$' || *it == ',' || *it == '%' || *it == '-'
              || *it == '_' || *it == '.' || *it == '!' || *it == '~' || *it == '*' || *it == '\'' || *it == '('
              || *it == ')') {
            ++it;
            if (it == uri.end() || *it == '#') {
              query_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
              if (*it == '#') {
                state = STATE_FRAGMENT;
                ++it;
                last_token = it;
              }
            }
          } else {
            throw UriParsingException();
          }
          break;
        }
        case STATE_FRAGMENT: {
          fragment_ = uri.substr(std::distance(uri.begin(), last_token));
          it = uri.end();
          break;
        }
      }
    }
  } catch (std::exception& e) {
    throw UriParsingException();
  }
  if ((state == STATE_HOST && host_.empty()) || (state == STATE_HOST_TYPE) || (state == STATE_PORT && port_.empty())
      || (type_ == TYPE_NONE)) {
    throw UriParsingException();
  }
}

enum Uri::type Uri::getType() const { return type_; }

std::string Uri::getUserInfo() const { return userinfo_; }

std::string Uri::getHost() const { return host_; }

std::string Uri::getPort() const { return port_; }

std::string Uri::getQuery() const { return query_; }

std::string Uri::getPath() const { return "/" + path_; }

std::string Uri::getRaw() const { return raw_; }

std::string Uri::getDecodedPath() const {
  std::string decoded = "/";
  for (std::string::const_iterator it = path_.begin(); it != path_.end(); ++it) {
    char c = *it;
    switch (c) {
      case '%':
        if (it + 1 != path_.end() && it + 2 != path_.end()) {
          char hs[3] = {it[1], it[2], 0};
          decoded += static_cast<char>(strtol(hs, nullptr, 16));
          it += 2;
        }
        break;
      case '+':
        decoded += ' ';
        break;
      default:
        decoded += c;
    }
  }
  return decoded;
}