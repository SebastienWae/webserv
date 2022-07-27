#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "HttpResponseStatus.h"
#include "ServerConfig.h"

class Config {
public:
  Config();
  Config& operator=(Config const& rhs);
  Config(Config const& src);
  ~Config();
  static std::string delcom(std::string const& str);
  static std::string openfile(const std::string& files);
  void checkconfig(const std::string& files);
  static void checkbracket(const std::string& str);
  static std::string checkextension(int argc, char** argv);
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
  class ConfException : public std::exception {
  public:
    virtual const char* what() const throw();
  };
  void setserver(std::string const& str, int* countserv);
  void seeklocation(std::string const& str, int* countserv);
  void setlocation(std::string const& str, const int* countserv);
  void parse(void);
  std::vector<ServerConfig> getServerConfig() const;

private:
  std::vector<ServerConfig> servers;
  std::vector<std::string> ser;
  std::vector<std::string> loca;
};

#endif
