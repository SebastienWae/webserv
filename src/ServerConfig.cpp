#include "ServerConfig.h"

#include <cstddef>
#include <exception>
#include <iostream>
#include <limits>
#include <vector>

#include "HttpResponseStatus.h"

ServerConfig::ServerConfig(std::string const& hostname, std::string const& port)
    : hostname_(hostname.empty() ? "localhost" : hostname),
      port_(port.empty() ? "80" : port),
      max_body_size_(std::numeric_limits<std::size_t>::max()) {}

ServerConfig::~ServerConfig() {
  for (std::vector<Route*>::iterator it = routes_.begin(); it != routes_.end(); ++it) {
    delete *it;
  }
  for (std::map<HttpResponseClientError::code, File*>::iterator it = client_errors_pages_.begin();
       it != client_errors_pages_.end(); ++it) {
    delete it->second;
  }
  for (std::map<HttpResponseServerError::code, File*>::iterator it = server_errors_pages_.begin();
       it != server_errors_pages_.end(); ++it) {
    delete it->second;
  }
}

ServerConfig::ParsingException::ParsingException(std::string const& msg) throw() : msg_(msg) {}
ServerConfig::ParsingException::~ParsingException() throw() {}
char const* ServerConfig::ParsingException::what() const throw() { return msg_.c_str(); }

Route* ServerConfig::parse(std::string const& line) {  // NOLINT
  std::string::size_type sep = line.find('=');
  if (sep != std::string::npos) {
    std::string key = line.substr(0, sep);
    std::string value = line.substr(sep + 1);

    if (key == "max_body_size") {
      if (value.empty()) {
        throw ParsingException("Config file error at line: " + line);
      }
      max_body_size_ = std::atoi(value.c_str());
      return NULL;
    }
    if (key == "error") {
      sep = value.find(' ');
      std::string code = value.substr(0, sep);
      std::string path = value.substr(sep + 1);

      File* file = new File(path);
      if (file->getType() == File::REG && file->isReadable() && file->getIStream() != NULL) {
        int code_i = std::atoi(code.c_str());
        if ((code_i - 400) >= 0 && (code_i - 400) < (418 - 400)) {  // NOLINT
          HttpResponseClientError::code code = static_cast<HttpResponseClientError::code>(code_i - 400);
          std::pair<HttpResponseClientError::code, File*> page(code, file);
          std::map<HttpResponseClientError::code, File*>::iterator dup = client_errors_pages_.find(code);
          if (dup != client_errors_pages_.end()) {
            delete dup->second;
            dup->second = file;
          } else {
            client_errors_pages_.insert(page);
          }
          return NULL;
        } else if ((code_i - 500) >= 0 && (code_i - 500) < (506 - 500)) {  // NOLINT
          HttpResponseServerError::code code = static_cast<HttpResponseServerError::code>(code_i - 500);
          std::pair<HttpResponseServerError::code, File*> page(code, file);
          std::map<HttpResponseServerError::code, File*>::iterator dup = server_errors_pages_.find(code);
          if (dup != server_errors_pages_.end()) {
            delete dup->second;
            dup->second = file;
          } else {
            server_errors_pages_.insert(page);
          }
          return NULL;
        } else {
          delete file;
        }
      } else {
        delete file;
      }
    }
    if (key == "route") {
      try {
        Uri uri(value);
        if (uri.getType() == Uri::TYPE_RELATIVE && uri.getQuery().empty()) {
          for (std::vector<Route*>::iterator it = routes_.begin(); it != routes_.end(); ++it) {
            if ((*it)->getLocation() == value) {
              return *it;
            }
          }
          Route* new_route = new Route(value);
          routes_.push_back(new_route);
          return new_route;
        }
        throw ParsingException("Config file error at line: " + line);
      } catch (std::exception& e) {
        throw ParsingException("Config file error at line: " + line);
      }
    }
  }
  throw ParsingException("Config file error at line: " + line);
}

void ServerConfig::verify() const throw(ParsingException) {
  if (hostname_.empty() || port_.empty()) {
    throw ParsingException("All servers must have an hostname and a port");
  }
  if (routes_.empty()) {
    throw ParsingException("All server must have at least one default route '/'");
  }
  for (std::vector<Route*>::const_iterator it = routes_.begin();;) {
    if ((*it)->getLocation() == "/") {
      break;
    }
    ++it;
    if (it == routes_.end()) {
      throw ParsingException("All server must have a default '/' route");
    }
  }
}

Route* ServerConfig::matchRoute(Uri const* uri) const {  // NOLINT
  int max = 0;
  Route* match = NULL;
  Route* defaut_match = NULL;

  std::string uri_path = uri->getPath();

  for (std::vector<Route*>::const_iterator it = routes_.begin(); it != routes_.end(); ++it) {
    std::string route_path = (*it)->getLocation();
    std::string::iterator uri_it = uri_path.begin();
    std::string::iterator route_it = route_path.begin();

    if (route_path == "/") {
      defaut_match = *it;
    }

    int c = 0;
    while (true) {
      if (*uri_it != *route_it) {
        if (uri_it == uri_path.end()) {
          c = 0;
          break;
        }
        if (route_it == route_path.end() && *uri_it == '/') {
          ++c;
          break;
        }
        c = 0;
        break;
      }
      if (route_it == route_path.end()) {
        return *it;
      }
      if (*route_it == '/') {
        ++c;
      }
      ++uri_it;
      ++route_it;
    }
    if (c > max) {
      max = c;
      match = *it;
    }
  }
  return match == NULL ? defaut_match : match;
}

std::string const& ServerConfig::getHostname() const { return hostname_; }
std::string const& ServerConfig::getPort() const { return port_; }

std::string ServerConfig::getHost() const { return hostname_ + ":" + port_; }

File* ServerConfig::getErrorPage(HttpResponseClientError::code code) const {
  std::map<HttpResponseClientError::code, File*>::const_iterator error_page = client_errors_pages_.find(code);
  if (error_page != client_errors_pages_.end()) {
    return error_page->second;
  }
  return NULL;
}

File* ServerConfig::getErrorPage(HttpResponseServerError::code code) const {
  std::map<HttpResponseServerError::code, File*>::const_iterator error_page = server_errors_pages_.find(code);
  if (error_page != server_errors_pages_.end()) {
    return error_page->second;
  }
  return NULL;
}

std::size_t ServerConfig::getMaxBodySize() const { return max_body_size_; }
