#ifndef POLL_ELEMENT_H
#define POLL_ELEMENT_H

#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>

#include <iostream>

#define POLL_INIT_NUMBER 5

class Client {
public:
  Client();
  ~Client();

  struct pollfd fd_info;

  int new_socket;
  sockaddr_storage client_address;
  socklen_t address_len;
  char remoteIP[INET6_ADDRSTRLEN];

  std::string response;
};

class PollElement {
public:
  PollElement();
  ~PollElement();

  Client* clients_array;

  int poll_fd_size;  // max size in the array - TO DO : check if overflows
  int active_fds;    // nb of fd currently used
  int poll_ret;      //  nb of fd in the array that have had an event occur

  void initPollElement(int listener);
  PollElement addToPollfds(int new_socket);
  // TO DO : void deleteFromPollfds(int index); undefined behaviour for now
};

#endif