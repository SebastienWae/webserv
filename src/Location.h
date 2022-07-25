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
  void setindex(const std::string &tmp);
  void setroot(const std::string &tmp);
  void setupload_store(const std::string &tmp);
  void setcgi_pass(const std::string &tmp);
  void setredirection(const std::string &tmp);
  void parseloc(void);
  void checkport(void);

private:
  std::string name;
  std::string allow;
  std::string index;
  std::string root;
  std::string upload_store;
  std::string cgi_pass;
  std::string redirection;  // return
};

#endif