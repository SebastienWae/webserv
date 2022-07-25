#include "Server.h"

server::server()
    : server_names("default"), error_page("default"), client_max_body_size("default"){};

server &server::operator=(server const &rhs) {
  if (this != &rhs) {
    this->port = rhs.port;
    this->server_names = rhs.server_names;
    this->error_page = rhs.error_page;
    this->client_max_body_size = rhs.client_max_body_size;
    this->location = rhs.location;
  }
  return (*this);
}

server::server(server const &src) { *this = src; };

server::~server(){};

void server::setport(const std::string &tmp) { this->port.push_back(tmp); }

void server::setserver_names(const std::string &tmp) { this->server_names = tmp; }

void server::seterror_page(const std::string &tmp) { this->error_page = tmp; }

void server::setclient_max_body_size(const std::string &tmp) { this->client_max_body_size = tmp; }

void server::setlocation(const Location &loc) { this->location.push_back(loc); }

void server::parseserv(void) {
  for (size_t i = 0; i < this->port.size(); i++) {
    if (this->port[i].compare(0, strlen("listen "), "listen ") == 0) {
      this->port[i].erase(0, strlen("listen "));
    }
  }
  if (this->server_names.compare(0, strlen("server_name "), "server_name ") == 0) {
    this->server_names.erase(0, strlen("server_name "));
  }
  if (this->error_page.compare(0, strlen("error_page "), "error_page ") == 0) {
    this->error_page.erase(0, strlen("error_page "));
  }
  if (this->client_max_body_size.compare(0, strlen("client_max_body_size "),
                                         "client_max_body_size ")
      == 0) {
    this->client_max_body_size.erase(0, strlen("client_max_body_size "));
  }
  for (size_t i = 0; i < this->location.size(); i++) {
    this->location[i].parseloc();
  }
}

void server::checkip(void) {
  std::vector<std::string> tmp;
  size_t start = 0;
  size_t end = 0;

  for (size_t i = 0; i < this->port.size(); i++) {
    if ((this->port[i].find(':', 0)) != std::string::npos) {
      for (size_t j = 0; j < this->port[i].size(); j++) {
        if ((this->port[i].compare(j, strlen("."), ".")) == 0) {
          end = j;
          tmp.push_back(this->port[i].substr(start, end - start));
          start = end + 1;
        }
        if ((this->port[i].compare(j, strlen(":"), ":")) == 0) {
          end = j;
          tmp.push_back(this->port[i].substr(start, end - start));
          start = end + 1;
          break;
        }
      }
    }
  }
  if (tmp.size() > 4) {
    throw server::IpException();
  }
  for (size_t i = 0; i < tmp.size(); i++) {
    for (size_t j = 0; j < tmp[i].size(); j++) {
      if (tmp[i][0] > '2' || tmp[i][0] < '0') {
        throw server::IpException();
      }
      if (tmp[i][0] == '2' && (tmp[i][1] > '5')) {
        throw server::IpException();
      }
      if (tmp[i][0] == '2' && (tmp[i][1] == '5') && (tmp[i][1] > '5')) {
        throw server::IpException();
      }
      if (tmp[i][j] > '9' || tmp[i][j] < '0') {
        throw server::IpException();
      }
    }
  }
}

void server::checkport(void) {
  size_t found;
  std::string str;
  for (size_t i = 0; i < this->port.size(); i++) {
    found = this->port[i].find(':', 0);
    if (found == std::string::npos) {
      if (this->port[i].size() > 5) {
        throw server::PortException();
      }
      if (this->port[i].size() == 5) {
        if (this->port[i][0] > '9' || this->port[i][0] < '0' || this->port[i][1] > '9'
            || this->port[i][1] < '0' || this->port[i][2] > '9' || this->port[i][2] < '0'
            || this->port[i][3] > '9' || this->port[i][3] < '0' || this->port[i][4] > '9'
            || this->port[i][4] < '0') {
          throw server::PortException();
        }
        if (this->port[i][0] == '6' || this->port[i][1] > '5') {
          throw server::PortException();
        }
        if (this->port[i][0] == '6' || this->port[i][0] == '5' || this->port[i][0] > '5') {
          throw server::PortException();
        }
        if (this->port[i][0] == '6' || this->port[i][0] == '5' || this->port[i][0] == '5'
            || this->port[i][0] > '3') {
          throw server::PortException();
        }
        if (this->port[i][0] == '6' || this->port[i][0] == '5' || this->port[i][0] == '5'
            || this->port[i][0] == '3' || this->port[i][0] > '5') {
          throw server::PortException();
        }
      } else {
      }
    }
  }
}

const char *server::IpException::what(void) const throw() { return ("Exception  : Bad IP"); }
const char *server::PortException::what(void) const throw() { return ("Exception  : Bad Port"); }
