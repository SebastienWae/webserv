#ifndef CGI_H
#define CGI_H
#include <iostream>
#include <map>

#include "Client.h"
#include "ServerConfig.h"

class Cgi {
public:
  Cgi(Client* client, ServerConfig const* server_config, std::string const& method);
  Cgi();
  ~Cgi();
  void executeCgi(int const& kq, Client* client);

private:
  std::map<std::string, std::string> env;
};

#endif