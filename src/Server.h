#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

#include <list>

#include "HttpResponse.h"
#include "ServerConfig.h"

#define MAX_PENDING_CONNECTIONS 50   // TO SET
#define BUFSIZE_CLIENT_REQUEST 4096  // TO SET

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
  void getClientRequest(int event_fd);
  class ClientGetRequestException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };
  void sendingMessageBackToClient(int event_fd, HttpResponse const& response);
  class ClientSendResponseException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };

  ServerConfig const* getConfig() const;
  /* TO DO : Setter and Getter to put these variables in private */
  std::string port;

private:
  /*kqueue variables */
  struct kevent change_event[4];
  struct kevent event[4];
  int new_events;
  int kq;

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
