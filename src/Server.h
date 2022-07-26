#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>

#include <list>

#include "HttpResponse.h"
#include "PollElement.h"
#include "ServerConfig.h"

/*queue size of pending connections. if full => sends
ECONNREFUSED or tries again to connect - depends on client */
#define MAX_PENDING_CONNECTIONS 50   // TO SET
#define BUFSIZE_CLIENT_REQUEST 1024  // TO SET

#define TIMEOUT 5000

class Server {
public:
  Server(ServerConfig const& config);
  ~Server();
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

  void* run();

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
  void checkClientConnexion();
  class ClientGetRequestException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };
  void sendingMessageBackToClient(int index, HttpResponse const& response);
  class ClientSendResponseException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };

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
  ServerConfig const& config_;
};

#endif
