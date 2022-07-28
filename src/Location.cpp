#include "Location.h"

#include <exception>

#include "Uri.h"

Location::Location()
    : allow("GET POST DELETE"),
      directory_page("default"),
      directory_listing("default"),
      directory_listing_bool(false),
      root("default"),
      upload_store("default"),
      cgi_pass("default"),
      redirection("default"){};

Location::~Location(){};
void Location::seturi(const std::string& tmp) {
  std::string tmp2 = tmp;
  if (tmp2.compare(0, strlen("location "), "location ") == 0) {
    tmp2.erase(0, strlen("location "));
  }
  Uri uri_(tmp2);
  if (uri_.getType() == Uri::TYPE_RELATIVE && uri_.getUserInfo().empty() && uri_.getQuery().empty()) {
    uri = uri_;
  } else {
    throw std::exception();
  }
}
void Location::setallow(const std::string& tmp) { this->allow = tmp; }
void Location::setdirectory_page(const std::string& tmp) { this->directory_page = tmp; }
void Location::setdirectory_listing(const std::string& tmp) { this->directory_listing = tmp; }
void Location::setroot(const std::string& tmp) { this->root = tmp; }
void Location::setupload_store(const std::string& tmp) { this->upload_store = tmp; }
void Location::setcgi_pass(const std::string& tmp) { this->cgi_pass = tmp; }
void Location::setredirection(const std::string& tmp) { this->redirection = tmp; }

Uri Location::geturi(void) const { return (this->uri); }
std::vector<enum Http::method> Location::getallowed(void) const { return (this->allowed); }
std::string Location::getdirectory_page(void) const { return (this->directory_page); }
bool Location::getdirectory_listing_bool(void) const { return (this->directory_listing_bool); }
std::string Location::getroot(void) const { return (this->root); }
std::string Location::getuploadstore(void) const { return (this->upload_store); }
std::string Location::getcgi_pass(void) const { return (this->cgi_pass); }
std::map<enum HttpResponseRedir::code, std::string> Location::getredir(void) const { return (this->redir); }

void Location::parseloc(void) {
  if (this->allow.compare(0, strlen("allow "), "allow ") == 0) {
    this->allow.erase(0, strlen("allow "));
  }
  if (this->directory_page.compare(0, strlen("directory_page "), "directory_page ") == 0) {
    this->directory_page.erase(0, strlen("directory_page "));
  }
  if (this->directory_listing.compare(0, strlen("directory_listing "), "directory_listing ") == 0) {
    this->directory_listing.erase(0, strlen("directory_listing "));
  }
  if ((this->directory_listing.compare(0, strlen("on"), "on")) == 0) {
    this->directory_listing_bool = true;
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
  if (this->redirection.compare(0, strlen("return "), "return ") == 0) {
    this->redirection.erase(0, strlen("return "));
  }
}
void Location::parseallow(void) {
  if (this->allow.find("GET", 0) != std::string::npos) {
    this->allowed.push_back(Http::GET);
  }
  if (this->allow.find("POST", 0) != std::string::npos) {
    this->allowed.push_back(Http::POST);
  }
  if (this->allow.find("DELETE", 0) != std::string::npos) {
    this->allowed.push_back(Http::DELETE);
  }
}

void Location::trimloc(void) {
  size_t found = std::string::npos;
  const std::string WHITESPACE = " \n\r\t\f\v";
  // found = this->name.find_first_of(WHITESPACE);
  // if (found != std::string::npos) {
  //   throw Location::TrimException();
  // }
  // found = this->allow.find_first_of(WHITESPACE);
  // found = this->index.find_first_of(WHITESPACE);
  found = this->root.find_first_of(WHITESPACE);
  // found = this->upload_store.find_first_of(WHITESPACE);
  // found = this->cgi_pass.find_first_of(WHITESPACE);
  // found = this->redirection.find_first_of(WHITESPACE);
  if (found != std::string::npos) {
    throw Location::TrimException();
  }
}
void Location::parseredir(void) {
  size_t found;
  std::string tmp;
  std::string tmp2;
  size_t size;
  found = this->redirection.find_first_of(' ', 0);
  tmp = this->redirection.substr(0, found);
  tmp2 = this->redirection.substr(found + 1, this->redirection.size());
  // conversion into int
  std::pair<enum HttpResponseRedir::code, std::string> p1;
  size_t n = tmp.length();
  char char_array[n + 1];
  strcpy(char_array, tmp.c_str());
  size = atoi(char_array);
  if ((size - 308) > 0 && (size - 300) < (308 - 300)) {
    p1.first = static_cast<enum HttpResponseRedir::code>(size - 300);
    p1.second = tmp2;
    this->redir.insert(p1);
  }
}
const char* Location::TrimException::what(void) const throw() { return ("Exception  : Trim error"); }