#include "Client.h"

#include <_types/_uint8_t.h>
#include <sys/_types/_pid_t.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <__nullptr>
#include <cstddef>
#include <cstring>
#include <vector>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpResponseStatus.h"
#include "Log.h"
#include "ServerConfig.h"

Client::Client(int socket, struct in_addr sin_addr)
    : socket_(socket),
      timestamp_(std::time(nullptr)),
      request_(NULL),
      response_(nullptr),
      reading_(true),
      replying_(false),
      replied_(false),
      ip_(sin_addr),
      child_(0) {}

Client::~Client() {
  if (child_ != 0) {
    kill(child_, SIGKILL);
  }
  if (response_ != nullptr) {
    delete response_;
  }
}

int Client::getSocket() const { return socket_; }

void Client::read(unsigned int bytes, Config const& config) throw(ReadException) {
  INFO("Reading data");

  std::vector<uint8_t> data(bytes, 0);

  std::size_t len = recv(socket_, reinterpret_cast<char*>(&data[0]), bytes, 0);

  if (len <= 0) {
    throw ReadException();
  }

  if (request_ != NULL) {
    ServerConfig const* sc = config.matchServerConfig(request_->getHost());
    request_->addChunk(data, sc->getMaxBodySize());
  } else {
    request_ = new HttpRequest(data, config);
    if (request_->getStatus() == HttpRequest::S_CONTINUE) {
      ServerConfig const* sc = config.matchServerConfig(request_->getHost());
      HttpResponse r(HttpResponseInfo::_100, sc);
      std::string r_continue = r.getHeaders();
      std::size_t len = ::send(socket_, r_continue.c_str(), r_continue.size(), 0);
      if (len < 0) {
        ERROR(std::strerror(errno));
        throw WriteException();
      }
    }
  }

  if (request_->getStatus() != HttpRequest::S_CONTINUE) {
    reading_ = false;
  }
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
    if (replying_) {
      try {
        std::vector<uint8_t> resp = response_->getContent(bytes);
        std::size_t len = ::send(socket_, reinterpret_cast<char*>(&resp[0]), bytes, 0);
        if (len < 0) {
          ERROR(std::strerror(errno));
          throw WriteException();
        }
        return;
      } catch (HttpResponse::EndOfResponseException& e) {
        replied_ = true;
        return;
      }
    } else {
      response = response_->getHeaders();
      replying_ = true;
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

void Client::setReponse(HttpResponse* response) { response_ = response; }

bool Client::isReading() const { return reading_; }
void Client::setRead() { reading_ = false; }

bool Client::hasReplied() const { return replied_; }
void Client::setReplied() { replied_ = true; }

void Client::setCGIPID(int child) { child_ = child; }
int Client::getCGIPID() const { return child_; }

std::time_t const& Client::getTime() const { return timestamp_; }