#ifndef CGI_H
#define CGI_H

#include <string>
#include <vector>

#include "Client.h"
#include "ServerConfig.h"

#define ENVLENGTH 16
#define ENVCOL 16

class CGI {
public:
  CGI(Client* client, ServerConfig const* server_config, File const* target, std::string const& method);
  ~CGI();

  void execute();

private:
  std::vector<std::string> env_;
  File const* target;
  Client* client_;
};

#endif