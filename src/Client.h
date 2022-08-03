#ifndef CLIENT_H
#define CLIENT_H

#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/socket.h>

#include <ctime>
#include <exception>

#include "HttpRequest.h"

class Client {
public:
  Client(int socket);

  ~Client();

  class ReadException : public std::exception {
    virtual const char* what() const throw() { return "Cannot read from client socket"; }
  };

  class WriteException : public std::exception {
    virtual const char* what() const throw() { return "Cannot write in client socket"; }
  };

  int getSocket() const;

  void read(unsigned int bytes) throw(ReadException);
  void send(unsigned int bytes) throw(WriteException);

  HttpRequest* getRequest() const;

  void setResponseData(std::string const& data);
  void addResponseData(std::string const& data);
  std::size_t getResponseSize() const;

  bool isReading() const;
  void setRead();

  bool hasReplied() const;
  void setReplied();

  std::time_t const& getTime() const;

private:
  int socket_;
  std::time_t timestamp_;
  HttpRequest* request_;
  std::string response_data_;
  bool reading_;
  bool replied_;
  struct in_addr ip_;
};

#endif
