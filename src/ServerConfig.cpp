#include "ServerConfig.h"

ServerConfig::ServerConfig() : server_names("default"), error_page("default"), client_max_body_size("default"){};

ServerConfig &ServerConfig::operator=(ServerConfig const &rhs) {
  if (this != &rhs) {
    this->listen = rhs.listen;
    this->server_names = rhs.server_names;
    this->error_page = rhs.error_page;
    this->client_max_body_size = rhs.client_max_body_size;
    this->location = rhs.location;
  }
  return (*this);
}

ServerConfig::ServerConfig(ServerConfig const &src) { *this = src; };

ServerConfig::~ServerConfig(){};

void ServerConfig::setlisten(const std::string &tmp) { this->listen.push_back(tmp); }

void ServerConfig::setserver_names(const std::string &tmp) { this->server_names = tmp; }

void ServerConfig::seterror_page(const std::string &tmp) { this->error_page = tmp; }

void ServerConfig::setclient_max_body_size(const std::string &tmp) { this->client_max_body_size = tmp; }

void ServerConfig::setlocation(const Location &loc) { this->location.push_back(loc); }

void ServerConfig::parseserv(void) {
  for (size_t i = 0; i < this->listen.size(); i++) {
    if (this->listen[i].compare(0, strlen("listen "), "listen ") == 0) {
      this->listen[i].erase(0, strlen("listen "));
    }
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
  for (size_t i = 0; i < this->location.size(); i++) {
    this->location[i].parseloc();
    this->location[i].parseallow();
    this->location[i].trimloc();
  }
}

void ServerConfig::checkip(void) {
  std::vector<std::string> tmp;
  size_t start = 0;
  size_t end = 0;

  for (size_t i = 0; i < this->listen.size(); i++) {
    if ((this->listen[i].find(':', 0)) != std::string::npos) {
      for (size_t j = 0; j < this->listen[i].size(); j++) {
        if ((this->listen[i].compare(j, strlen("."), ".")) == 0) {
          end = j;
          tmp.push_back(this->listen[i].substr(start, end - start));
          start = end + 1;
        }
        if ((this->listen[i].compare(j, strlen(":"), ":")) == 0) {
          end = j;
          tmp.push_back(this->listen[i].substr(start, end - start));
          start = end + 1;
          this->ip.push_back(this->listen[i].substr(0, end));
          this->port.push_back(this->listen[i].substr(end + 1, this->listen[i].size()));
          break;
        }
      }
    }
  }
  for (size_t i = 0; i < this->listen.size(); i++) {
    if ((this->listen[i].find(':', 0)) == std::string::npos) {
      this->port.push_back(this->listen[i].substr(0, this->listen[i].size()));
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
    if (this->port[i].empty()) {
      throw ServerConfig::PortException();
    }
    for (size_t i = 0; i < this->port.size(); i++) {
      if (this->port[i].size() > 5) {
        throw ServerConfig::PortException();
      }
    }
    if (this->port[i].size() == 1) {
      if (this->port[i][0] > '9' || this->port[i][0] < '0') {
        throw ServerConfig::PortException();
      }
    }
    if (this->port[i].size() == 2) {
      if (this->port[i][0] > '9' || this->port[i][0] < '0' || this->port[i][1] > '9' || this->port[i][1] < '0') {
        throw ServerConfig::PortException();
      }
    }
    if (this->port[i].size() == 3) {
      if (this->port[i][0] > '9' || this->port[i][0] < '0' || this->port[i][1] > '9' || this->port[i][1] < '0'
          || this->port[i][2] > '9' || this->port[i][2] < '0') {
        throw ServerConfig::PortException();
      }
    }
    if (this->port[i].size() == 4) {
      if (this->port[i][0] > '9' || this->port[i][0] < '0' || this->port[i][1] > '9' || this->port[i][1] < '0'
          || this->port[i][2] > '9' || this->port[i][2] < '0' || this->port[i][3] > '9' || this->port[i][3] < '0') {
        throw ServerConfig::PortException();
      }
    }
    if (this->port[i].size() == 5) {
      if (this->port[i][0] > '9' || this->port[i][0] < '0' || this->port[i][1] > '9' || this->port[i][1] < '0'
          || this->port[i][2] > '9' || this->port[i][2] < '0' || this->port[i][3] > '9' || this->port[i][3] < '0'
          || this->port[i][4] > '9' || this->port[i][4] < '0') {
        throw ServerConfig::PortException();
      }
      if (this->port[i][0] == '6' && this->port[i][1] > '5') {
        throw ServerConfig::PortException();
      }
      if (this->port[i][0] == '6' && this->port[i][1] == '5' && this->port[i][2] > '5') {
        throw ServerConfig::PortException();
      }
      if (this->port[i][0] == '6' && this->port[i][1] == '5' && this->port[i][2] == '5' && this->port[i][3] > '3') {
        throw ServerConfig::PortException();
      }
      if (this->port[i][0] == '6' && this->port[i][1] == '5' && this->port[i][2] == '5' && this->port[i][3] == '3'
          && this->port[i][4] > '5') {
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
}
const char *ServerConfig::IpException::what(void) const throw() { return ("Exception  : Bad IP"); }
const char *ServerConfig::TrimservException::what(void) const throw() { return ("Exception  : Trimserv"); }
const char *ServerConfig::PortException::what(void) const throw() { return ("Exception  : Bad Port"); }
