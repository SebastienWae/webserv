#include "Config.h"

#include <__nullptr>

config::config() {
  keyword.push_back("server");
  keyword.push_back("port");
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
    this->Server = rhs.Server;
  }
  return (*this);
}
config::config(config const& src) { *this = src; }
config::~config() {}
void config::checkconfig(const std::string& files) {
  (void)files;
  std::vector<std::string> split;
  std::ifstream inputFile;
  std::string str;
  size_t found = 0;
  size_t foundserv = 0;
  bool flag = true;
  int count = 0;
  int count2 = 0;
  std::stringstream strings;
  std::list<std::string>::iterator it = this->keyword.begin();
  std::string buf;
  inputFile.open("nginx.conf");
  if (inputFile.fail()) {
    throw config::FilesException();
  }
  strings << inputFile.rdbuf();
  str = strings.str();
  found = str.find('{', found);
  for (std::string::iterator ite = &str[found]; ite != str.end(); it++) {
    if (*ite == '{') {
      count2++;
    }
    if (*ite == '}') {
      count2--;
    }
    if (count2 == 0) {
    }
  }

  servername(str);
  foundserv = str.find(*it, foundserv);
  it++;
  while (it != this->keyword.end()) {
    while (flag) {
      found = str.find(*it, found);
      if (found == std::string::npos) {
        flag = false;
      } else {
        this->Server[count].port = 0;
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

const char* config::FilesException::what(void) const throw() {
  return ("Exception  : Fail open files");
}

void config::servername(std::string const& str) {
  size_t start = 0;
  size_t end = 0;
  bool flag = true;
  std::string name = "server_name";

  while (flag) {
    start = str.find(name, start);
    if (start == std::string::npos) {
      flag = false;
    } else {
      end = start;
      while (str[end] != ';') {
        end++;
      }
      server* serv = new server;
      this->Server.push_back(*serv);
      start += name.length();
    }
  }
  str.find("server_name", start);
}
