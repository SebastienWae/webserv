#ifndef SERVER_H
#define SERVER_H
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

private:
  std::vector<std::string> port;
  std::string server_names;
  std::string error_page;
  std::string client_max_body_size;
  std::vector<Location> location;
};
#endif