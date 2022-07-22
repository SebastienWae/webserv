#ifndef LOCATION_H
#define LOCATION_H

#include <iostream>

class Location {
public:
  Location();
  Location &operator=(Location const &rhs);
  Location(Location const &src);
  ~Location();

private:
  std::string name;
  std::string allow;
  std::string autoindex;
  std::string upload_store;
  std::string cgi_pass;
  std::string redirection;  // return
};

#endif