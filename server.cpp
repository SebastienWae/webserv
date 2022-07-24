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
