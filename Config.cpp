#include "Config.h"

#include <__nullptr>
#INCLUDE

config::config() {
  keyword.push_back("server");
  keyword.push_back("listen");
  keyword.push_back("server_name");
  keyword.push_back("error_page");
  keyword.push_back("client_max_body_size");
  keyword.push_back("location");
  keyword.push_back("allow");
  keyword.push_back("autoindex");
  keyword.push_back("upload_store");
  keyword.push_back("cgi_pass");
  keyword.push_back("return");
}
config& config::operator=(config const& rhs) {
  if (this != &rhs) {
    this->server = rhs.server;
  }
  return (*this);
}
config::config(config const& src) { *this = src; }
config::~config() {}
void config::checkconfig(const std::string& files) {
  (void)files;
  std::ifstream inputFile;
  std::string str;
  size_t found = 0;
  size_t foundserv = 0;
  bool flag = true;
  std::stringstream strings;
  std::list<std::string>::iterator it = this->keyword.begin();
  std::string buf;
  // std::string nameserv = "server";
  // double countserv = 0;
  inputFile.open("nginx.conf");
  if (inputFile.fail()) {
    throw config::FilesException();
  }
  strings << inputFile.rdbuf();
  str = strings.str();
  foundserv = str.find(*it, foundserv);
  while (found != std::string::npos) {
    it++;
    while (it != this->keyword.end()) {
      while (flag) {
        found = str.find(*it, found);
        if (found == std::string::npos) {
          flag = false;
        } else {
          found += (*it).length();
        }
      }
      it++;
      flag = true;
    }
    it = this->keyword.begin();
    foundserv += (*it).length();
    foundserv = str.find(*it, foundserv);
  }
}

const char* config::FilesException::what(void) const throw() {
  return ("Exception  : Fail open files");
}
