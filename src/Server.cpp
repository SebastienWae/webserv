#include "Server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <exception>

#include "HttpResponse.h"
#include "HttpResponseStatus.h"

#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#  include <string.h>
#endif

Server::Server(ServerConfig const* config) : config_(config){};

Server::~Server(){};

char const* Server::ServerCoreFatalException::what() const throw() { return "Server core fatal error."; }
char const* Server::ServerCoreNonFatalException::what() const throw() { return "Server error."; }
char const* Server::ListenerException::what() const throw() { return "Listener error."; }
char const* Server::ClientGetRequestException::what() const throw() { return "Error while getting client request."; }
char const* Server::ClientSendResponseException::what() const throw() {
  return "Error while sending response to client.";
}

void* Server::getAddress(struct sockaddr* sockaddress) {
  if (sockaddress->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sockaddress)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sockaddress)->sin6_addr);
}

// If try to read from non-blocking socket without data => not allowed to
// block—it => return -1 + errno set to EAGAIN or EWOULDBLOCK
void Server::startListening() {
  if (listen(listener, MAX_PENDING_CONNECTIONS) < 0) {
    std::cerr << "Listen failed" << std ::endl;
    throw ListenerException();
  }
  FD_SET(listener, &master);
  fd_max = listener;
  if (fcntl(listener, F_SETFL, O_NONBLOCK) == -1) {
    std::cerr << "Cannot set fd to non blocking." << std ::endl;
    throw ListenerException();
  }
}

void Server::createListenerSocket() {
  int y = 1;
  memset((char*)&hints, 0, sizeof(hints));

  hints.ai_socktype = SOCK_STREAM;
#if defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
  hints.ai_family = AF_INET;
  hints.ai_flags = 0;
#elif defined(__APPLE__) && defined(__MACH__)
  hints.ai_family = AF_UNSPEC;
  hints.ai_flags = AI_PASSIVE;
#endif

  if (getaddrinfo(NULL, (config_->getport()).c_str(), &hints, &address_info) != 0) {
    std::cerr << "Getaddrinfo error." << std ::endl;
    throw ListenerException();
  }
  for (p = address_info; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_flags);
    if (listener < 0) {
      continue;
    }
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener);
      continue;
    }
    break;
  }
  freeaddrinfo(address_info);
  if (p == NULL) {
    std::cerr << "Bind failed" << std ::endl;
    throw ListenerException();
  }
};

void Server::getClientRequest() {
  address_len = sizeof(client_address);
  if ((new_socket = accept(listener, (struct sockaddr*)&client_address, (socklen_t*)&address_len)) < 0) {
    std::cerr << "Accept error" << std ::endl;
    throw ClientGetRequestException();
  }
  if (fcntl(new_socket, F_SETFL, O_NONBLOCK) == -1) {
    std::cerr << "Cannot set new socket to non blocking." << std ::endl;
    throw ClientGetRequestException();
  }
  FD_SET(new_socket, &master);
  if (new_socket > fd_max) {
    fd_max = new_socket;
  }

  std::cout << std::endl
            << "*** New connection from "
            << inet_ntop(client_address.ss_family, getAddress((struct sockaddr*)&client_address), remoteIP,
                         INET6_ADDRSTRLEN)
            << " ***" << std::endl
            << std::endl;
}

void Server::sendingMessageBackToClient(int index) {
  char buffer[BUFSIZE_CLIENT_REQUEST];  // TO SET
  long int ret_recv = recv(index, buffer, BUFSIZE_CLIENT_REQUEST, 0);
  if (ret_recv <= 0) {
    if (ret_recv == 0) {
      std::cerr << std::endl << "Connection closed by client." << std::endl;
    } else if (ret_recv < 0) {
      std::cerr << std::endl << "Error: No byte to read." << std::endl;
    }
    close(index);
    FD_CLR(index, &master);

  } else {
    buffer[ret_recv] = '\0';
    std::cout << "Received from client : " << std ::endl << std::endl << buffer << std::endl;
    // TO DO : select response
    HttpResponse response(HttpResponseSuccess::_200, "<html><body><h1>hello</h1></body></html>", "text/html", config_);

    std::string raw_response = response.getRaw();

    std::cout << "Response sent to client : " << raw_response << std::endl << std::endl;
    send(index, (raw_response + "ok").c_str(), raw_response.length() + 2, 0);
    std::cout << std::endl << "*** Message sent to client ***" << std::endl;
  }
}

void* Server::run() {
  try {
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    createListenerSocket();
    startListening();
    while (true) {
      try {
        read_fds = master;
        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) {
          throw ServerCoreNonFatalException();  // a changer?
        }
        for (int i = 0; i <= fd_max; i++) {
          if (FD_ISSET(i, &read_fds)) {
            if (i == listener) {
              getClientRequest();
            } else {
              sendingMessageBackToClient(i);
            }
          }
        }
      } catch (ServerCoreNonFatalException& e) {  // a changer?
        std::cout << e.what() << std::endl;
      }
    }
    close(listener);
  } catch (ServerCoreFatalException& e) {
    std::cerr << "FATAL ERROR - SERVER STOPPED LISTENING ON PORT " << config_->getport() << std::endl;
  }
  return NULL;
}

void* threadWrapper(void* current) { return (static_cast<Server*>(current))->run(); };

ServerConfig const* Server::getConfig() const { return (config_); };
