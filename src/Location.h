#ifndef LOCATION_H
#define LOCATION_H

#include <exception>
#include <iostream>
#include <vector>

#include "Http.h"

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
  void trimloc(void);
  void parseallow(void);
  class TrimException : public std::exception {
  public:
    virtual const char *what() const throw();
  };

private:
  std::string name;
  std::string allow;
  std::vector<enum Http::method> allowed;
  std::string index;
  std::string root;
  std::string upload_store;
  std::string cgi_pass;
  std::string redirection;  // return
};

#endif