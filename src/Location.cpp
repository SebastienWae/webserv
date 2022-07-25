#include "Location.h"

Location::Location()
    : name("default"),
      allow("default"),
      index("default"),
      root("default"),
      upload_store("default"),
      cgi_pass("default"),
      redirection("default"){};
Location& Location::operator=(Location const& rhs) {
  if (this != &rhs) {
    this->name = rhs.name;
    this->allow = rhs.allow;
    this->index = rhs.index;
    this->root = rhs.root;
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
void Location::setindex(const std::string& tmp) { this->index = tmp; }
void Location::setroot(const std::string& tmp) { this->root = tmp; }
void Location::setupload_store(const std::string& tmp) { this->upload_store = tmp; }
void Location::setcgi_pass(const std::string& tmp) { this->cgi_pass = tmp; }
void Location::setredirection(const std::string& tmp) { this->redirection = tmp; }
void Location::parseloc(void) {
  if (this->name.compare(0, strlen("location "), "location ") == 0) {
    this->name.erase(0, strlen("location "));
  }

  if (this->allow.compare(0, strlen("allow "), "allow ") == 0) {
    this->allow.erase(0, strlen("allow "));
  }
  if (this->index.compare(0, strlen("index "), "index ") == 0) {
    this->index.erase(0, strlen("index "));
  }
  if (this->root.compare(0, strlen("root "), "root ") == 0) {
    this->root.erase(0, strlen("root "));
  }
  if (this->upload_store.compare(0, strlen("upload_store "), "upload_store ") == 0) {
    this->upload_store.erase(0, strlen("upload_store "));
  }
  if (this->cgi_pass.compare(0, strlen("cgi_pass "), "cgi_pass ") == 0) {
    this->cgi_pass.erase(0, strlen("cgi_pass "));
  }
  if (this->redirection.compare(0, strlen("redirection "), "redirection ") == 0) {
    this->redirection.erase(0, strlen("redirection "));
  }
}