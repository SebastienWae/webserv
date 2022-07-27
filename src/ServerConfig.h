#ifndef SERVERCONFIG_H
#define SERVERCONFIG_H
#include <exception>
#include <list>
#include <map>
#include <vector>

#include "HttpResponseStatus.h"
#include "Location.h"

class ServerConfig {
public:
  ServerConfig();
  ServerConfig &operator=(ServerConfig const &rhs);
  ServerConfig(ServerConfig const &src);
  ~ServerConfig();
  void setlisten(const std::string &tmp);
  void setserver_names(const std::string &tmp);
  void seterror_page(const std::string &tmp);
  void setclient_max_body_size(const std::string &tmp);
  void setlocation(const Location &loc);
  void setroot(const std::string &tmp);
  void setport(const std::string &tmp);
  void setauth(const std::string &tmp);
  void parseserv(void);
  void checkip(void);
  void checkport(void);
  void trimserv(void);
  void parserror(void);
  void splitauth(void);
  std::string getlisten(void);
  std::string getroot(void);
  std::string getport(void);
  std::string getserver_names(void);
  std::map<enum HttpResponseClientError::code, std::string> getclienterror(void);
  std::map<enum HttpResponseServerError::code, std::string> getservererror(void);
  std::pair<std::string, std::string> getauthpair(void);
  int getsize(void);
  std::vector<Location> getlocation(void);

  class IpException : public std::exception {
  public:
    virtual const char *what() const throw();
  };

  class PortException : public std::exception {
  public:
    virtual const char *what() const throw();
  };
  class TrimservException : public std::exception {
  public:
    virtual const char *what() const throw();
  };

private:
  std::string listen;
  std::string root;
  std::string port;
  std::string server_names;
  std::string error_page;
  std::map<enum HttpResponseClientError::code, std::string> clienterror;
  std::map<enum HttpResponseServerError::code, std::string> servererror;
  std::string client_max_body_size;
  std::string auth;
  std::pair<std::string, std::string> authpair;
  int max_size;
  std::vector<Location> location;
};
#endif