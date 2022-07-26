#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H
#include <cstring>
#include <exception>
#include <list>
#include <vector>

#include "Location.h"

class ServerConfig {
public:
  ServerConfig();
  ServerConfig &operator=(ServerConfig const &rhs);
  ServerConfig(ServerConfig const &src);
  ~ServerConfig();
  void setlisten(const std::string &tmp);
  void setserver_names(const std::string &tmp);
  void seterror_page(const std::string &tmp);
  void setclient_max_body_size(const std::string &tmp);
  void setlocation(const Location &loc);
  void parseserv(void);
  void checkip(void);
  void checkport(void);
  void trimserv(void);
  class IpException : public std::exception {
  public:
    virtual const char *what() const throw();
  };
  class PortException : public std::exception {
  public:
    virtual const char *what() const throw();
  };
  class TrimservException : public std::exception {
  public:
    virtual const char *what() const throw();
  };
  std::string getPort() const;

private:
  std::vector<std::string> listen;
  std::vector<std::string> ip;
  std::vector<std::string> port;
  std::string host;
  std::string server_names;
  std::string error_page;
  std::string client_max_body_size;
  std::vector<Location> location;
};
#endif