#ifndef ROUTE_H
#define ROUTE_H

#include <map>
#include <set>
#include <string>

#include "Http.h"
#include "HttpResponseStatus.h"
#include "Uri.h"

class Route {
public:
  Route(std::string const& location);
  ~Route();

  class ParsingException : public std::exception {
  public:
    ParsingException(std::string const& msg) throw();
    ~ParsingException() throw();
    virtual const char* what() const throw();

  private:
    std::string msg_;
  };

  void parse(std::string const& line);
  void verify() const;

  std::string const& getLocation() const;
  std::string const& getRoot() const;
  std::string const& getDirectoryPage() const;
  std::string const& getUploadStore() const;

  bool isRedirection() const;
  std::pair<HttpResponseRedir::code, Uri*> const& getRedirection() const;

  bool isAllowedMethod(enum Http::method method) const;

  std::string matchCGI(std::string const& file) const;

private:
  std::string const location_;
  std::string root_;
  bool directory_listing_;
  std::string directory_page_;
  std::string upload_store_;
  std::pair<HttpResponseRedir::code, Uri*> redirection_;
  std::set<enum Http::method> allowed_methods_;
  std::map<std::string, std::string> cgi_;
};

#endif
