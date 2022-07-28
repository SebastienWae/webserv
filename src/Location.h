#ifndef LOCATION_H
#define LOCATION_H

#include <exception>
#include <iostream>
#include <map>
#include <vector>

#include "Http.h"
#include "HttpResponseStatus.h"
#include "Uri.h"

class Location {
public:
  Location();
  ~Location();
  void seturi(const std::string &tmp);
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
  Uri geturi(void) const;
  std::vector<enum Http::method> getallowed(void) const;
  std::string getdirectory_page(void) const;
  bool getdirectory_listing_bool(void) const;
  std::string getroot(void) const;
  std::string getuploadstore(void) const;
  std::string getcgi_pass(void) const;
  std::map<enum HttpResponseRedir::code, std::string> getredir(void) const;

  class TrimException : public std::exception {
  public:
    virtual const char *what() const throw();
  };

private:
  Uri uri;
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