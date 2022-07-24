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
  void setserver(std::string const& str, int* countserv);
  void seeklocation(std::string const& str, int* countserv);
  void setlocation(std::string const& str, const int* countserv);

private:
  std::vector<server> Server;
  std::string ser[5];
  std::string Loca[7];
};

#endif
