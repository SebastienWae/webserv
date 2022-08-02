#ifndef CONFIG_H
#define CONFIG_H

#include <cstddef>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include "HttpRequest.h"
#include "ServerConfig.h"

class Config {
public:
  Config(std::string const& config_path);
  ~Config();

  class ParsingException : public std::exception {
  public:
    ParsingException(std::string const& msg) throw();
    ~ParsingException() throw();
    virtual const char* what() const throw();

  private:
    std::string msg_;
  };
  void checkPort(std::string const& port);
  void checkHostname(std::string const& hostname);
  void parse(std::ifstream& file);

  ServerConfig const* matchServerConfig(HttpRequest const* request) const;

  std::set<std::string> getPorts() const;

private:
  enum parse_state { S_NONE, S_IN_SERVER };

  std::vector<ServerConfig*> servers_;
};

#endif
