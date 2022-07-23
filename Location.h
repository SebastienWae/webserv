#ifndef LOCATION_H
#define LOCATION_H

#include <iostream>

class Location {
public:
  Location();
  Location &operator=(Location const &rhs);
  Location(Location const &src);
  ~Location();
  void setname(const std::string &tmp);
  void setallow(const std::string &tmp);
  void setautoindex(const std::string &tmp);
  void setupload_store(const std::string &tmp);
  void setcgi_pass(const std::string &tmp);
  void setredirection(const std::string &tmp);

private:
  std::string name;
  std::string allow;
  std::string autoindex;
  std::string upload_store;
  std::string cgi_pass;
  std::string redirection;  // return
};

#endif