#include "Client.h"

#include <sys/_types/_pid_t.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include "HttpRequest.h"
#include "Log.h"

Client::Client(int socket, struct in_addr sin_addr)
    : socket_(socket),
      timestamp_(std::time(nullptr)),
      request_(NULL),
      reading_(true),
      replied_(false),
      ip_(sin_addr),
      child_(0) {}

Client::~Client() {
  if (child_ != 0) {
    kill(child_, SIGKILL);
  }
}

int Client::getSocket() const { return socket_; }

void Client::read(unsigned int bytes) throw(ReadException) {
  INFO("Reading data");

  char* data = new char[bytes + 1];
  bzero(data, bytes + 1);
  std::size_t len = recv(socket_, data, bytes, 0);

  if (len <= 0) {
    delete[] data;
    throw ReadException();
  }

  if (request_ != NULL) {
    request_->addChunk(data);
  } else {
    request_ = new HttpRequest(data);
  }

  if (request_->getStatus() != HttpRequest::S_CONTINUE) {
    reading_ = false;
  }

  delete[] data;
}

void Client::send(unsigned int bytes) throw(WriteException) {
  INFO("Sending data");

  if (child_ != 0) {
    int status = 0;
    pid_t wait = waitpid(child_, &status, WNOHANG);
    if (wait == -1 || wait != 0) {
      replied_ = true;
    }
  } else {
    std::string response;
    if (bytes == 0 || bytes >= response_data_.size()) {
      response = response_data_;
      replied_ = true;
    } else {
      response = response_data_.substr(0, bytes);
    }
    std::size_t len = ::send(socket_, response.c_str(), response.size(), 0);
    if (len < 0) {
      ERROR(std::strerror(errno));
      throw WriteException();
    }
  }
}

HttpRequest* Client::getRequest() const { return request_; }

struct in_addr Client::getIp() const {
  return ip_;
}

void Client::setResponseData(std::string const& data) { response_data_ = data; }
void Client::addResponseData(std::string const& data) { response_data_ += data; }
std::size_t Client::getResponseSize() const { return response_data_.size(); }

bool Client::isReading() const { return reading_; }
void Client::setRead() { reading_ = false; }

bool Client::hasReplied() const { return replied_; }
void Client::setReplied() { replied_ = true; }

void Client::setCGIPID(int child) { child_ = child; }
int Client::getCGIPID() const { return child_; }

std::time_t const& Client::getTime() const { return timestamp_; }