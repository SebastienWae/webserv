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
  CGI(Client* client, ServerConfig const* server_config, std::pair<std::string, File*> const& cgi_pair,
      std::string const& method);
  ~CGI();

  class CGIException : public std::exception {
  public:
    CGIException(std::string const& msg) throw();
    ~CGIException() throw();
    virtual const char* what() const throw();

  private:
    std::string msg_;
  };

  void execute();

private:
  std::vector<std::string> env_;
  File const* target_;
  Client* client_;
  std::string cwd_;
};

#endif