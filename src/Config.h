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
#define variable 8
class config {
public:
  config();
  config& operator=(config const& rhs);
  config(config const& src);
  ~config();
  std::string delcom(std::string const& str);
  std::string openfile(const std::string& files);
  void checkconfig(const std::string& files);
  void checkbracket(const std::string& str);
  std::string checkextension(int argc, char** argv);
  class FilesException : public std::exception {
  public:
    virtual const char* what() const throw();
  };
  class ArgException : public std::exception {
  public:
    virtual const char* what() const throw();
  };
  class BadException : public std::exception {
  public:
    virtual const char* what() const throw();
  };
  class CommaException : public std::exception {
  public:
    virtual const char* what() const throw();
  };
  class NameException : public std::exception {
  public:
    virtual const char* what() const throw();
  };
  class BracketException : public std::exception {
  public:
    virtual const char* what() const throw();
  };
  void setserver(std::string const& str, int* countserv);
  void seeklocation(std::string const& str, int* countserv);
  void setlocation(std::string const& str, const int* countserv);
  void parse(void);

private:
  std::vector<server> Server;
  std::string ser[5];
  std::string Loca[variable];
};

#endif
