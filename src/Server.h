#ifndef SERVER_H
#define SERVER_H
#include <exception>
#include <list>
#include <vector>

#include "Location.h"

class server {
public:
  server();
  server &operator=(server const &rhs);
  server(server const &src);
  ~server();
  void setport(const std::string &tmp);
  void setserver_names(const std::string &tmp);
  void seterror_page(const std::string &tmp);
  void setclient_max_body_size(const std::string &tmp);
  void setlocation(const Location &loc);
  void parseserv(void);
  void checkip(void);
  void checkport(void);
  class IpException : public std::exception {
  public:
    virtual const char *what() const throw();
  };
  class PortException : public std::exception {
  public:
    virtual const char *what() const throw();
  };

private:
  std::vector<std::string> port;
  std::string server_names;
  std::string error_page;
  std::string client_max_body_size;
  std::vector<Location> location;
};
#endif