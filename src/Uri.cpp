#include "Uri.h"

#include <algorithm>
#include <cctype>
#include <iterator>
#include <stdexcept>
#include <string>

// NOLINTNEXTLINE
Uri::Uri(std::string const& uri) throw(Uri::UriParsingException) : type_(Uri::TYPE_NONE) {
  enum parse_state state = STATE_IDENTIFY;
  std::string::const_iterator last_token;
  for (std::string::const_iterator it = uri.begin(); it != uri.end();) {
    switch (state) {
      case STATE_IDENTIFY: {
        if (uri.at(0) == '/') {
          type_ = Uri::TYPE_RELATIVE;
          state = STATE_PATH;
          ++it;
        } else if (std::isalpha(uri.at(0)) != 0) {
          state = STATE_SCHEME;
        } else {
          throw UriParsingException();
        }
        break;
      }
      case STATE_SCHEME: {
        if (std::isalnum(*it) != 0 || *it == '+' || *it == '-' || *it == '+') {
          ++it;
        } else if (*it == ':' && std::distance(uri.begin(), it) > 0) {
          scheme_ = uri.substr(0, std::distance(uri.begin(), it));
          std::transform(scheme_.begin(), scheme_.end(), scheme_.begin(), ::tolower);
          ++it;
          if (*it == '/' && *(it + 1) == '/') {
            type_ = Uri::TYPE_ABSOLUTE;
            it += 2;
            std::size_t dist = std::distance(uri.begin(), it);
            std::string::size_type at = uri.find('@', dist);
            std::string::size_type slash = uri.find('/', dist);
            if (at != std::string::npos && (slash == std::string::npos || at < slash)) {
              state = STATE_USERINFO;
            } else {
              state = STATE_HOSTNAME;
            }
          } else {
            type_ = Uri::TYPE_RELATIVE;
            state = STATE_PATH;
          }
          last_token = it;
        } else {
          throw UriParsingException();
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
          std::size_t dist = std::distance(uri.begin(), last_token);
          std::string::size_type slash = uri.find('/', dist);
          std::string host = uri.substr(dist, slash);
          std::string::size_type dot = host.find_last_of('.');
          if (dot != std::string::npos && (std::isalpha(host.at(dot + 1)) != 0)) {
            state = STATE_HOSTNAME;
          } else {
            state = STATE_IPV4ADDRESS;
          }
        } else {
          throw UriParsingException();
        }
        break;
      }
      case STATE_HOSTNAME: {
        if (std::isalnum(*it) != 0 || *it == '.' || *it == '-') {
          ++it;
          if (it == uri.end() || *it == '/' || *it == ':' || *it == '#') {
            host_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
            std::transform(host_.begin(), host_.end(), host_.begin(), ::tolower);
            if (*it == '/' || *it == ':' || *it == '#') {
              if (*it == '/') {
                state = STATE_PATH;
              } else if (*it == ':') {
                state = STATE_PORT;
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
          if (it == uri.end() || *it == '/' || *it == ':' || *it == '#') {
            host_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
            if (*it == '/' || *it == ':' || *it == '#') {
              if (*it == '/') {
                state = STATE_PATH;
              } else if (*it == ':') {
                state = STATE_PORT;
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
          if (it == uri.end() || *it == '/' || *it == '#') {
            port_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
            if (*it == '/' || *it == '#') {
              if (*it == '/') {
                state = STATE_PATH;
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
        if (std::isalnum(*it) != 0 || *it == ':' || *it == '@' || *it == '&' || *it == '=' || *it == '+' || *it == '$'
            || *it == ',' || *it == '%' || *it == '-' || *it == '_' || *it == '!' || *it == '~' || *it == '*'
            || *it == '\'' || *it == '(' || *it == ')') {
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
        } else {
          throw UriParsingException();
        }
        break;
      }
      case STATE_QUERY: {
        if (std::isalnum(*it) != 0 || *it == ';' || *it == '/' || *it == '?' || *it == ':' || *it == '@' || *it == '&'
            || *it == '=' || *it == '+' || *it == '$' || *it == ',' || *it == '%' || *it == '-' || *it == '_'
            || *it == '.' || *it == '!' || *it == '~' || *it == '*' || *it == '\'' || *it == '(' || *it == ')') {
          ++it;
          if (it == uri.end() || *it == '#') {
            query_ = uri.substr(std::distance(uri.begin(), last_token), std::distance(last_token, it));
            if (*it == '#') {
              state = STATE_FRAGMENT;
              ++it;
              last_token = it;
            }
          }
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
}

enum Uri::type Uri::getType() const { return type_; }

std::string Uri::getUserInfo() const { return userinfo_; }

std::string Uri::getHost() const { return host_; }

std::string Uri::getPort() const { return port_; }

std::string Uri::getQuery() const { return query_; }

std::string Uri::getRaw() const {
  return (scheme_.empty() ? "" : scheme_ + "://") + (userinfo_.empty() ? "" : userinfo_ + "@") + host_
         + (port_.empty() ? "" : ":" + port_) + (path_.empty() ? "" : "/" + path_)
         + (query_.empty() ? "" : "?" + query_) + (fragment_.empty() ? "" : "#" + fragment_);
}