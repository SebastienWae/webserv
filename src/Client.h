#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

#include <cstring>
#include <iostream>

#define BUFSIZE_CLIENT_REQUEST 1024  // TO SET => max size necessary for a request ?

class Client {
public:
  Client();
  Client(Client const& other);
  Client& operator=(Client const& other);
  ~Client();

  struct pollfd fd_info;

  int new_socket;
  sockaddr_storage client_address;
  socklen_t address_len;
  char remoteIP[INET6_ADDRSTRLEN];

  char request[BUFSIZE_CLIENT_REQUEST];
  std::string response;
};

#endif