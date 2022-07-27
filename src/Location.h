#ifndef LOCATION_H
#define LOCATION_H

#include <exception>
#include <iostream>
#include <map>
#include <vector>

#include "Http.h"
#include "HttpResponseStatus.h"

class Location {
public:
  Location();
  Location &operator=(Location const &rhs);
  Location(Location const &src);
  ~Location();
  void setname(const std::string &tmp);
  void setallow(const std::string &tmp);
  void setdirectory_page(const std::string &tmp);
  void setdirectory_listing(const std::string &tmp);
  void setroot(const std::string &tmp);
  void setupload_store(const std::string &tmp);
  void setcgi_pass(const std::string &tmp);
  void setredirection(const std::string &tmp);
  void parseloc(void);
  void trimloc(void);
  void parseallow(void);
  void parseredir(void);
  std::string getname(void);
  std::vector<enum Http::method> getallowed(void);
  std::string getdirectory_page(void);
  bool getdirectory_listing_bool(void);
  std::string getroot(void);
  std::string getuploadstore(void);
  std::string getcgi_pass(void);
  std::map<enum HttpResponseRedir::code, std::string> getredir(void);

  class TrimException : public std::exception {
  public:
    virtual const char *what() const throw();
  };

private:
  std::string name;
  std::string allow;
  std::vector<enum Http::method> allowed;
  std::string directory_page;
  std::string directory_listing;
  bool directory_listing_bool;
  std::string root;
  std::string upload_store;
  std::string cgi_pass;
  std::string redirection;  // return
  std::map<enum HttpResponseRedir::code, std::string> redir;
};

#endif