#ifndef CGI_H
#define CGI_H
#include <iostream>
#include <map>

#include "Client.h"
#include "ServerConfig.h"
#define ENVLENGTH 16
#define ENVCOL 16

class Cgi {
public:
  Cgi(Client* client, ServerConfig const* server_config, std::string const& method);
  Cgi();
  ~Cgi();
  void executeCgi(int const& kq, Client* client);

private:
  std::vector<std::string> env;
};

#endif