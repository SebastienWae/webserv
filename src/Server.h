#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>

#include <list>

#include "HttpResponse.h"
#include "ServerConfig.h"

/*queue size of pending connections. if full => sends
ECONNREFUSED or tries again to connect - depends on client */
#define MAX_PENDING_CONNECTIONS 50   // TO SET
#define BUFSIZE_CLIENT_REQUEST 1024  // TO SET

#define TIMEOUT 5000

/* to use for multiple ports => multithread */
void* threadWrapper(void* current);
class Server {
public:
  Server(ServerConfig const* config);
  ~Server();
  class ServerCoreFatalException : public std::exception {
  public:
    virtual char const* what() const throw();
  };

  class ServerCoreNonFatalException : public std::exception {
  public:
    virtual char const* what() const throw();
  };

  void* run();

  /* Listener functions */
  void createListenerSocket();
  void startListening();
  class ListenerException : public ServerCoreFatalException {
  public:
    virtual char const* what() const throw();
  };

  /* Client functions */
  static void* getAddress(struct sockaddr* sockaddress);
  void getClientRequest();
  class ClientGetRequestException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };
  void sendingMessageBackToClient(int index);
  class ClientSendResponseException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };

  ServerConfig const* getConfig() const;
  /* TO DO : Setter and Getter to put these variables in private */
  std::string port;

private:
  /* Element containing all needed things to execute select */
  fd_set master;
  fd_set read_fds;
  int fd_max;

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
  ServerConfig const* config_;
};

#endif
