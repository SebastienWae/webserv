#ifndef OnePort_H
#define OnePort_H

#include <netdb.h>

#include <exception>
#include <list>

#include "PollElement.h"

/*queue size of pending connections. if full => sends
ECONNREFUSED or tries again to connect - depends on client */
#define MAX_PENDING_CONNECTIONS 50  // TO SET

class OnePort {
public:
  OnePort();
  ~OnePort();

  /* Exceptions */
  class ServerCoreFatalException : public std::exception {
  public:
    virtual char const* what() const throw();
  };

  class ServerCoreNonFatalException : public std::exception {
  public:
    virtual char const* what() const throw();
  };

  /* to use for multiple ports => multithread */
  static void* launchHelper(void* current);

  void* launchOnOnePort();

  /* Listener functions */
  void createListenerSocket();
  void startListening() const;
  class ListenerException : public ServerCoreFatalException {
  public:
    virtual char const* what() const throw();
  };

  /* Poll functions */
  void pollProcessInit();
  class PollException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };

  /* Client functions */
  static void* getAddress(struct sockaddr* sockaddress);
  void getClientRequest(int i);
  class ClientGetRequestException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };
  void getResponse(int i) const;
  void sendMessageBackToClient(int index) const;
  class ClientSendResponseException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };

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
};

#endif