#ifndef ROUTE_H
#define ROUTE_H

#include <map>
#include <set>

#include "File.h"
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

  class NotFoundException : public std::exception {};
  class ForbiddenException : public std::exception {};

  void parse(std::string const& line);
  void verify() const;

  std::string const& getLocation() const;
  File* getRoot() const;
  File* getDirecoryPage() const;
  File* getUploadStore() const;

  bool isRedirection() const;
  std::pair<HttpResponseRedir::code, Uri*> const& getRedirection() const;

  bool isDirectoryListing() const;

  bool isAllowedMethod(enum Http::method method) const;

  File* matchFile(Uri const* uri) const;
  std::pair<std::string, File*> matchCGI(Uri const* uri) const;
  File* matchFileUpload(Uri const* uri) const;

private:
  std::string const location_;
  File* root_;
  bool directory_listing_;
  File* directory_page_;
  File* upload_store_;
  std::pair<HttpResponseRedir::code, Uri*> redirection_;
  std::set<enum Http::method> allowed_methods_;
  std::map<std::string, File*> cgi_;
};

#endif
