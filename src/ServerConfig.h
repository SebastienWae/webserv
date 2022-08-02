#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "File.h"
#include "Route.h"

class ServerConfig {
public:
  ServerConfig(std::string const& hostname, std::string const& port);
  ~ServerConfig();

  class ParsingException : public std::exception {
  public:
    ParsingException(std::string const& msg) throw();
    ~ParsingException() throw();
    virtual const char* what() const throw();

  private:
    std::string msg_;
  };

  Route* parse(std::string const& line);
  void verify() const throw(ParsingException);

  Route* matchRoute(Uri const& uri) const;

  std::string const& getHostname() const;
  std::string const& getPort() const;
  std::string getHost() const;

  File* getErrorPage(HttpResponseClientError::code code) const;
  File* getErrorPage(HttpResponseServerError::code code) const;

  std::size_t getMaxBodySize() const;

private:
  std::string const hostname_;
  std::string const port_;
  std::size_t max_body_size_;
  std::vector<Route*> routes_;
  std::map<HttpResponseClientError::code, File*> client_errors_pages_;
  std::map<HttpResponseServerError::code, File*> server_errors_pages_;
};
#endif
