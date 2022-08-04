#ifndef CGI_H
#define CGI_H
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include <iostream>
#include <map>
#include <string>

#include "Client.h"
#include "Route.h"
#include "ServerConfig.h"
#define ENVLENGTH 16
#define ENVCOL 16

class Cgi {
public:
  Cgi(Client* client, ServerConfig const* server_config, std::string const& method);
  ~Cgi();
  void executeCgi(std::string const& path);

private:
  std::vector<std::string> env;
  Client* client_;
};

#endif