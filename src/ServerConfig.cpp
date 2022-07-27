#include "ServerConfig.h"

#include <utility>
ServerConfig::ServerConfig()
    : listen("default"),
      root("default"),
      port("80"),
      server_names("default"),
      error_page("default"),
      client_max_body_size("default"),
      auth("default"),
      max_size(0){};

ServerConfig &ServerConfig::operator=(ServerConfig const &rhs) {
  if (this != &rhs) {
    this->listen = rhs.listen;
    this->root = rhs.root;
    this->port = rhs.port;
    this->auth = rhs.auth;
    this->authpair = rhs.authpair;
    this->server_names = rhs.server_names;
    this->error_page = rhs.error_page;
    this->client_max_body_size = rhs.client_max_body_size;
    this->location = rhs.location;
  }
  return (*this);
}

ServerConfig::ServerConfig(ServerConfig const &src) { *this = src; };

ServerConfig::~ServerConfig(){};

void ServerConfig::setlisten(const std::string &tmp) { this->listen = tmp; }

void ServerConfig::setserver_names(const std::string &tmp) { this->server_names = tmp; }

void ServerConfig::seterror_page(const std::string &tmp) { this->error_page = tmp; }

void ServerConfig::setroot(const std::string &tmp) { this->root = tmp; }

void ServerConfig::setport(const std::string &tmp) { this->port = tmp; }

void ServerConfig::setauth(const std::string &tmp) { this->auth = tmp; }

void ServerConfig::setclient_max_body_size(const std::string &tmp) { this->client_max_body_size = tmp; }

void ServerConfig::setlocation(const Location &loc) { this->location.push_back(loc); }

std::string ServerConfig::getlisten(void) const { return (this->listen); }

std::string ServerConfig::getroot(void) const { return (this->root); }

std::string ServerConfig::getport(void) const { return (this->port); }

std::string ServerConfig::getserver_names(void) const { return (this->server_names); }

std::map<enum HttpResponseClientError::code, std::string> ServerConfig::getclienterror(void) const {
  return (this->clienterror);
}

std::map<enum HttpResponseServerError::code, std::string> ServerConfig::getservererror(void) const {
  return (this->servererror);
}
std::pair<std::string, std::string> ServerConfig::getauthpair(void) const { return (this->authpair); }

int ServerConfig::getsize(void) const { return (this->max_size); }

std::vector<Location> ServerConfig::getlocation(void) const { return (this->location); }

void ServerConfig::parseserv(void) {
  if (this->listen.compare(0, strlen("listen "), "listen ") == 0) {
    this->listen.erase(0, strlen("listen "));
  }
  if (this->root.compare(0, strlen("root "), "root ") == 0) {
    this->root.erase(0, strlen("root "));
  }
  if (this->port.compare(0, strlen("port "), "port ") == 0) {
    this->port.erase(0, strlen("port "));
  }

  if (this->server_names.compare(0, strlen("server_name "), "server_name ") == 0) {
    this->server_names.erase(0, strlen("server_name "));
  }
  if (this->error_page.compare(0, strlen("error_page "), "error_page ") == 0) {
    this->error_page.erase(0, strlen("error_page "));
  }
  if (this->client_max_body_size.compare(0, strlen("client_max_body_size "), "client_max_body_size ") == 0) {
    this->client_max_body_size.erase(0, strlen("client_max_body_size "));
  }
  if (this->auth.compare(0, strlen("auth "), "auth ") == 0) {
    this->auth.erase(0, strlen("auth "));
  }
  for (size_t i = 0; i < this->location.size(); i++) {
    this->location[i].parseloc();
    this->location[i].parseallow();
    this->location[i].trimloc();
    this->location[i].parseredir();
  }
}

void ServerConfig::checkip(void) {
  std::vector<std::string> tmp;
  size_t start = 0;
  size_t end = 0;

  for (size_t i = 0; i < this->listen.size(); i++) {
    if ((this->listen.compare(i, strlen("."), ".")) == 0) {
      end = i;
      tmp.push_back(this->listen.substr(start, end - start));
      start = end + 1;
    }
  }
  if (tmp.size() > 4) {
    throw ServerConfig::IpException();
  }
  for (size_t i = 0; i < tmp.size(); i++) {
    for (size_t j = 0; j < tmp[i].size(); j++) {
      if (tmp[i][0] > '2' || tmp[i][0] < '0') {
        throw ServerConfig::IpException();
      }
      if (tmp[i][0] == '2' && (tmp[i][1] > '5')) {
        throw ServerConfig::IpException();
      }
      if (tmp[i][0] == '2' && (tmp[i][1] == '5') && (tmp[i][1] > '5')) {
        throw ServerConfig::IpException();
      }
      if (tmp[i][j] > '9' || tmp[i][j] < '0') {
        throw ServerConfig::IpException();
      }
    }
  }
}

void ServerConfig::checkport(void) {
  std::string str;
  for (size_t i = 0; i < this->port.size(); i++) {
    for (size_t i = 0; i < this->port.size(); i++) {
      if (this->port.size() > 5) {
        throw ServerConfig::PortException();
      }
    }
    if (this->port.size() == 1) {
      if (this->port[0] > '9' || this->port[0] < '0') {
        throw ServerConfig::PortException();
      }
    }
    if (this->port.size() == 2) {
      if (this->port[0] > '9' || this->port[0] < '0' || this->port[1] > '9' || this->port[1] < '0') {
        throw ServerConfig::PortException();
      }
    }
    if (this->port.size() == 3) {
      if (this->port[0] > '9' || this->port[0] < '0' || this->port[1] > '9' || this->port[1] < '0'
          || this->port[2] > '9' || this->port[2] < '0') {
        throw ServerConfig::PortException();
      }
    }
    if (this->port.size() == 4) {
      if (this->port[0] > '9' || this->port[0] < '0' || this->port[1] > '9' || this->port[1] < '0'
          || this->port[2] > '9' || this->port[2] < '0' || this->port[3] > '9' || this->port[3] < '0') {
        throw ServerConfig::PortException();
      }
    }
    if (this->port.size() == 5) {
      if (this->port[0] > '9' || this->port[0] < '0' || this->port[1] > '9' || this->port[1] < '0'
          || this->port[2] > '9' || this->port[2] < '0' || this->port[3] > '9' || this->port[3] < '0'
          || this->port[4] > '9' || this->port[4] < '0') {
        throw ServerConfig::PortException();
      }
      if (this->port[0] == '6' && this->port[1] > '5') {
        throw ServerConfig::PortException();
      }
      if (this->port[0] == '6' && this->port[1] == '5' && this->port[2] > '5') {
        throw ServerConfig::PortException();
      }
      if (this->port[0] == '6' && this->port[1] == '5' && this->port[2] == '5' && this->port[3] > '3') {
        throw ServerConfig::PortException();
      }
      if (this->port[0] == '6' && this->port[1] == '5' && this->port[2] == '5' && this->port[3] == '3'
          && this->port[4] > '5') {
        throw ServerConfig::PortException();
      }
    }
  }
}
void ServerConfig::trimserv(void) {
  size_t found = std::string::npos;
  const std::string WHITESPACE = " \n\r\t\f\v";
  found = this->client_max_body_size.find_first_of(WHITESPACE);
  if (found != std::string::npos) {
    throw ServerConfig::TrimservException();
  }
  found = this->auth.find_first_of(WHITESPACE);
  if (found != std::string::npos) {
    throw ServerConfig::TrimservException();
  }
  size_t n = this->client_max_body_size.length();
  char char_array[n + 1];
  strcpy(char_array, this->client_max_body_size.c_str());
  this->max_size = atoi(char_array);
}
void ServerConfig::parserror(void) {
  size_t found;
  std::string tmp;
  std::string tmp2;
  size_t size;
  found = this->error_page.find_first_of(' ', 0);
  tmp = this->error_page.substr(0, found);
  tmp2 = this->error_page.substr(found + 1, this->error_page.size());
  // conversion into int
  std::pair<enum HttpResponseClientError::code, std::string> p1;
  std::pair<enum HttpResponseServerError::code, std::string> p2;
  size_t n = tmp.length();
  char char_array[n + 1];
  strcpy(char_array, tmp.c_str());
  size = atoi(char_array);
  if ((size - 400) > 0 && (size - 400) < (418 - 400)) {
    p1.first = static_cast<HttpResponseClientError::code>(size - 400);
    p1.second = tmp2;
    this->clienterror.insert(p1);
  } else if ((size - 500) > 0 && (size - 500) < (506 - 500)) {
    p2.first = static_cast<enum HttpResponseServerError::code>(size - 500);
    p2.second = tmp2;
    this->servererror.insert(p2);
  }
}
void ServerConfig::splitauth(void) {
  size_t found;
  found = this->auth.find_first_of(':', 0);
  this->authpair.first = this->auth.substr(0, found);
  this->authpair.second = this->auth.substr(found + 1, this->auth.size());
}

const char *ServerConfig::IpException::what(void) const throw() { return ("Exception  : Bad IP"); }
const char *ServerConfig::TrimservException::what(void) const throw() { return ("Exception  : Trimserv"); }
const char *ServerConfig::PortException::what(void) const throw() { return ("Exception  : Bad Port"); }
