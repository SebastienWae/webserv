#ifndef CLIENT_H
#define CLIENT_H

#include <ctime>
#include <exception>

#include "HttpRequest.h"

class Client {
public:
  Client(int socket);

  ~Client();

  int getSocket() const;

  void read(unsigned int bytes) throw(std::exception);
  void send(unsigned int bytes) throw(std::exception);

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
};

#endif
