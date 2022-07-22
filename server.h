#ifndef SERVER_H
#define SERVER_H
#include <list>
#include <vector>

#include "Location.h"

class server {
public:
  server();
  server& operator=(server const& rhs);
  server(server const& src);
  ~server();

private:
  int port;
  std::string server_names;
  std::string error_page;
  int client_max_body_size;
  std::list<Location> location;
};
#endif