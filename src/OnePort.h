#ifndef OnePort_H
#define OnePort_H

#include <netdb.h>
#include <iostream>
#include <list>
#include "PollElement.h"

/*queue size of pending connections. if full => sends
ECONNREFUSED or tries again to connect - depends on client */
#define MAX_PENDING_CONNECTIONS 50   // TO SET
#define BUFSIZE_CLIENT_REQUEST 1024  // TO SET

class OnePort {
 public:
  OnePort();
  ~OnePort();

  /* to use for multiple ports => multithread */
  static void* launchHelper(void* current);

  void* launchOnOnePort();

  /* Listener functions */
  void createListenerSocket();
  void startListening() const;

  /* Poll functions */
  void pollProcessInit();
  /* Client functions */
  static void* getAddress(struct sockaddr* sockaddress);
  void getClientRequest();
  void sendingMessageBackToClient(int index) const;

  /* TO DO : Setter and Getter to put these variables in private */
  std::string port;

 private:
  /* Element containing all needed things to execute poll */
  PollElement poll_elem;

  /* Listener variables */
  int listener;
  struct addrinfo hints;
  struct addrinfo* address_info;
  struct addrinfo* p;

  /* Client request variables */
  int new_socket;
  sockaddr_storage client_address;
  socklen_t address_len;
  char remoteIP[INET6_ADDRSTRLEN];
};

#endif