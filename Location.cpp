#include "Location.h"

Location::Location()
    : name("default"),
      allow("default"),
      autoindex("default"),
      upload_store("default"),
      cgi_pass("default"),
      redirection("default"){};
Location &Location::operator=(Location const &rhs) {
  if (this != &rhs) {
    this->name = rhs.name;
    this->allow = rhs.allow;
    this->autoindex = rhs.autoindex;
    this->upload_store = rhs.upload_store;
    this->cgi_pass = rhs.cgi_pass;
    this->redirection = rhs.redirection;
  }
  return (*this);
}
Location::Location(Location const &src) { *this = src; }
Location::~Location(){};