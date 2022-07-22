#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <__nullptr>
#include <exception>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Server.h"
class config {
public:
  config();
  config& operator=(config const& rhs);
  config(config const& src);
  ~config();
  void checkconfig(const std::string& files);
  void setconfig(const std::string& files);
  void checkbracket(std::string);
  class FilesException : public std::exception {
  public:
    virtual const char* what() const throw();
  };
  void servername(std::string const& str);

private:
  std::vector<server> Server;
  std::list<std::string> keyword;
};

#endif
