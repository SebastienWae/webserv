#include "Server.h"

server::server() : port(0), server_names("default"), error_page("default"){};
server& server::operator=(server const& rhs) {
  if (this != &rhs) {
    this->server_names = rhs.server_names;
    this->error_page = rhs.error_page;
    this->client_max_body_size = rhs.client_max_body_size;
    this->location = rhs.location;
  }
  return (*this);
}
server::server(server const& src) { *this = src; };
server::~server(){};