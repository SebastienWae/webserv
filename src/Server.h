#ifndef SERVER_H
#define SERVER_H

#include <netdb.h>
#include <sys/event.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <exception>
#include <iterator>
#include <set>
#include <vector>

#include "Client.h"
#include "Config.h"
#include "ServerConfig.h"

#define KQ_SIZE 1024
#define INVALID_SOCKET -1
#define TIMEOUT 30

class Server {
public:
  Server(Config const& config);
  ~Server();

  void start();

private:
  Config const& config_;

  int const kq_;
  struct kevent events_[KQ_SIZE];

  std::vector<int> ports_socket_;

  std::map<int, Client*> clients_;

  void updateEvents(int ident, short filter, u_short flags);
  void listenToPort(std::string const& port);
  void closeConnection(int socket);
  void acceptConnection(int socket);

  Client* findClient(int socket);
  void removeClient(Client* client);

  void processRequest(Client* client);

  void getHandler(Client* client, ServerConfig const* server_config);
  void headHandler(Client* client, ServerConfig const* server_config);
  void postHandler(Client* client, ServerConfig const* server_config);
  void deleteHandler(Client* client, ServerConfig const* server_config);

  void timeoutClient(Client* client);
};

#endif
