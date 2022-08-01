#include "Client.h"

#include <sys/socket.h>

#include <cstring>
#include <ctime>
#include <exception>

#include "HttpRequest.h"
#include "Log.h"

Client::Client(int socket)
    : socket_(socket), timestamp_(std::time(nullptr)), request_(NULL), reading_(true), replied_(false) {}

Client::~Client() {}

int Client::getSocket() const { return socket_; }

void Client::read(unsigned int bytes) throw(std::exception) {
  INFO("Reading data");

  char* data = new char[bytes + 1];
  bzero(data, bytes + 1);
  std::size_t len = recv(socket_, data, bytes, 0);

  if (len <= 0) {
    delete[] data;
    throw std::exception();
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

void Client::send(unsigned int bytes) throw(std::exception) {
  INFO("Sending data");

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
    throw std::exception();
  }
}

HttpRequest* Client::getRequest() const { return request_; }

void Client::setResponseData(std::string const& data) { response_data_ = data; }
void Client::addResponseData(std::string const& data) { response_data_ += data; }
std::size_t Client::getResponseSize() const { return response_data_.size(); }

bool Client::isReading() const { return reading_; }
void Client::setRead() { reading_ = false; }

bool Client::hasReplied() const { return replied_; }
void Client::setReplied() { replied_ = true; }

std::time_t const& Client::getTime() const { return timestamp_; }