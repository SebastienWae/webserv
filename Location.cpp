#include "Location.h"

Location::Location()
    : name("default"),
      allow("default"),
      autoindex("default"),
      upload_store("default"),
      cgi_pass("default"),
      redirection("default"){};
Location& Location::operator=(Location const& rhs) {
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
Location::Location(Location const& src) { *this = src; }
Location::~Location(){};
void Location::setname(const std::string& tmp) { this->name = tmp; }
void Location::setallow(const std::string& tmp) { this->allow = tmp; }
void Location::setautoindex(const std::string& tmp) { this->autoindex = tmp; }
void Location::setupload_store(const std::string& tmp) { this->upload_store = tmp; }
void Location::setcgi_pass(const std::string& tmp) { this->cgi_pass = tmp; }
void Location::setredirection(const std::string& tmp) { this->redirection = tmp; }