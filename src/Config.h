#ifndef CONFIG_H
#define CONFIG_H

#include "ServerConfig.h"

class Config {
public:
  class ParsingException : public std::exception {
  public:
    ParsingException() throw();
    ParsingException(std::string const& msg) throw();
    ~ParsingException() throw();
    virtual const char* what() const throw();

  private:
    std::string msg_;
  };

  Config(std::string const& config_path);
  ~Config();

  void parse();

  std::set<std::string> getPorts() const;

  ServerConfig const* matchServerConfig(std::string const& host) const;

private:
  enum parse_state { S_NONE, S_IN_SERVER };

  std::vector<ServerConfig*> servers_;
  File file_;
};

#endif
