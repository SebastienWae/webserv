#include "Server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <exception>

#include "HttpResponse.h"
#include "HttpResponseStatus.h"

Server::Server(ServerConfig const* config) : config_(config){};

Server::~Server() { delete[] poll_elem.poll_fds; };

char const* Server::ServerCoreFatalException::what() const throw() { return "Server core fatal error."; }
char const* Server::ServerCoreNonFatalException::what() const throw() { return "Server error."; }
char const* Server::ListenerException::what() const throw() { return "Listener error."; }
char const* Server::PollException::what() const throw() { return "Poll error."; }
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
void Server::startListening() const {
  if (listen(listener, MAX_PENDING_CONNECTIONS) < 0) {
    std::cerr << "Listen failed" << std ::endl;
    throw ListenerException();
  }
  // if (fcntl(listener, F_SETFL, O_NONBLOCK) == -1) {
  //   std::cerr << "Cannot set fd to non blocking." << std ::endl;
  //   throw ListenerException();
  // }
}

void Server::createListenerSocket() {
  int y = 1;
  memset((char*)&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, (config_->getport()).c_str(), &hints, &address_info) != 0) {
    std::cerr << "Getaddrinfo error." << std ::endl;
    throw ListenerException();
  }
  for (p = address_info; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_flags);
    if (listener < 0) {
      continue;
    }
    setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &y, sizeof(int));
    if (fcntl(listener, F_SETFL, O_NONBLOCK) == -1) {
      std::cerr << "Cannot set fd to non blocking." << std ::endl;
      throw ListenerException();
    }
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

void Server::pollProcessInit() {
  std::cout << "POLL" << std::endl;
  poll_elem.poll_ret = poll(&poll_elem.poll_fds[0], poll_elem.active_fds, 1);
  if (poll_elem.poll_ret == -1) {
    std::cerr << "Poll error." << std ::endl;
    throw PollException();
  }
  if (poll_elem.poll_ret == 0) {
    // std::cout << "Poll timed out" << std ::endl;
    // close connection with client when timeout ?
  }
}

/* Called when data is ready to be received */
void Server::getClientRequest() {
  address_len = sizeof(client_address);
  std::cout << "ACCEPT" << std::endl;
  if ((new_socket = accept(listener, (struct sockaddr*)&client_address, (socklen_t*)&address_len)) < 0) {
    std::cerr << "Accept error" << std ::endl;
    std::cout << strerror(errno) << std::endl;
  }
  if (fcntl(new_socket, F_SETFL, O_NONBLOCK) == -1) {
    std::cerr << "Cannot set newfd to non blocking." << std ::endl;
  }
  poll_elem = poll_elem.addToPollfds(new_socket);
  std::cout << "INET" << std::endl;
  std::cout << std::endl
            << "*** New connection from "
            << inet_ntop(client_address.ss_family, getAddress((struct sockaddr*)&client_address), remoteIP,
                         INET6_ADDRSTRLEN)
            << " ***" << std::endl
            << std::endl;
}

void Server::sendingMessageBackToClient(int index, HttpResponse const& response) {
  char buffer[BUFSIZE_CLIENT_REQUEST];  // TO SET
  std::cout << "READ" << std::endl;
  long int ret_recv = read(poll_elem.poll_fds[index].fd, buffer, BUFSIZE_CLIENT_REQUEST);

  if (ret_recv == 0) {
    std::cerr << std::endl << "Error: Connection closed by client" << std::endl;
    poll_elem = poll_elem.removeFromPollfds(index);
    throw ClientSendResponseException();
  }
  if (ret_recv < 0) {
    std::cerr << std::endl << "Error: No byte to read" << std::endl;
    poll_elem = poll_elem.removeFromPollfds(index);
    throw ClientSendResponseException();
  }
  buffer[ret_recv] = '\0';

  std::cout << "Received from client : " << std ::endl << std::endl << buffer << std::endl;
  std::string raw_response = response.getRaw();
  std::cout << "Response sent to client : " << raw_response << std::endl << std::endl;
  send(poll_elem.poll_fds[index].fd, (raw_response + "ok").c_str(), raw_response.length() + 2, 0);
  std::cout << std::endl << "*** Message sent to client ***" << std::endl;
}

void* Server::run() {
  try {
    createListenerSocket();
    startListening();
    poll_elem.initPollElement(listener);

    // std::cout << std::endl
    //           << "-----------> READY TO START ON PORT " << config_.getport() << " <-----------" << std::endl
    //           << std::endl;

    while (true) {
      pollProcessInit();
      for (int i = 0; i < poll_elem.poll_fd_size; i++) {
        try {
          if ((poll_elem.poll_fds[i].revents & POLLIN) != 0) {
            if (poll_elem.poll_fds[i].fd == listener) {
              getClientRequest();
            } else {
              HttpResponse resp(HttpResponseSuccess::_200, "<html><body><h1>hello</h1></body></html>", "text/html",
                                config_);
              sendingMessageBackToClient(i, resp);
              poll_elem.removeFromPollfds(i);
            }
          }
        } catch (ServerCoreNonFatalException& e) {
          std::cerr << e.what() << std::endl;
        }
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
