#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/ip.h>
#include <signal.h>

#include <ctime>

#include "Config.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class Client {
public:
  Client(int socket, struct in_addr sin_addr);

  ~Client();

  class ReadException : public std::exception {
    virtual const char* what() const throw() { return "Cannot read from client socket"; }
  };

  class WriteException : public std::exception {
    virtual const char* what() const throw() { return "Cannot write in client socket"; }
  };

  int getSocket() const;

  void read(unsigned int bytes, Config const& config) throw(ReadException);
  void send(unsigned int bytes) throw(WriteException);

  HttpRequest* getRequest() const;
  struct in_addr getIp() const;

  void setReponse(HttpResponse* response);

  bool isReading() const;
  void setRead();

  bool hasReplied() const;
  void setReplied();

  void setCGIPID(int child);
  int getCGIPID() const;

  std::time_t const& getTime() const;

private:
  int socket_;
  std::time_t timestamp_;
  HttpRequest* request_;
  HttpResponse* response_;
  bool reading_;
  bool replying_;
  bool replied_;
  struct in_addr ip_;
  int child_;
};

#endif
