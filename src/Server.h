#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <sys/event.h>
#include <sys/time.h>
#include <sys/types.h>

#include <list>
#include <vector>

#include "Config.h"
#include "ServerConfig.h"

#define MAX_PENDING_CONNECTIONS 50   // TO SET
#define BUFSIZE_CLIENT_REQUEST 4096  // TO SET

class Server {
public:
  Server(Config& servers_list);
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
  void createListenersSocket(ServerConfig const& server);

  class ListenerException : public ServerCoreNonFatalException {
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
  static std::string getClientPort(char buffer[BUFSIZE_CLIENT_REQUEST]);
  const ServerConfig* getCurrentConfig(std::string& current_port);
  void sendingMessageBackToClient(int event_fd);
  class ClientSendResponseException : public ServerCoreNonFatalException {
  public:
    virtual char const* what() const throw();
  };

  ServerConfig const* getConfig() const;

  void closeListeners();

private:
  /*kqueue variables */
  struct kevent change_event[4];
  struct kevent event[4];
  int new_events;
  int kq;

  /* Listener variables */
  std::vector<int> listeners;
  std::vector<ServerConfig> configs;

  /* Client request variables */
  int new_socket;
  sockaddr_storage client_address;
  socklen_t address_len;
  char remoteIP[INET6_ADDRSTRLEN];

  Config servers_list_;
};

#endif
