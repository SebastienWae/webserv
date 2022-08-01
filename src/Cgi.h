#ifndef CGI_H
#define CGI_H
#include <iostream>
#include <map>

class Cgi {
public:
  Cgi();
  ~Cgi();

private:
  std::map<std::string, std::string> env;
};

#endif