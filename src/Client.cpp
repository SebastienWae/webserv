#include "Client.h"

Client::Client() {}

Client& Client::operator=(Client const& other) {
  if (&other == this) {
    return (*this);
  }
  fd_info = other.fd_info;
  new_socket = other.new_socket;
  client_address = other.client_address;
  address_len = other.address_len;
  strcpy(remoteIP, other.remoteIP);
  strcpy(request, other.request);
  response = other.response;
  return (*this);
};

Client::Client(Client const& other) {
  fd_info = other.fd_info;
  new_socket = other.new_socket;
  client_address = other.client_address;
  address_len = other.address_len;
  strcpy(remoteIP, other.remoteIP);
  strcpy(request, other.request);
  response = other.response;
};

Client::~Client() {}
