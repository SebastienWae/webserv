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

Server::~Server(){};

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

void Server::startListening() const {
  if (listen(listener, MAX_PENDING_CONNECTIONS) < 0) {
    std::cerr << "Listen failed" << std ::endl;
    throw ListenerException();
  }
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
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
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

void Server::getClientRequest(int event_fd) {
  address_len = sizeof(client_address);
  new_socket = accept(event_fd, (struct sockaddr*)&client_address, (socklen_t*)&address_len);
  if (new_socket == -1) {
    perror("Accept socket error");
  }
  if (fcntl(new_socket, F_SETFL, O_NONBLOCK) == -1) {
    std::cerr << "Cannot set new socket to non blocking." << std ::endl;
    throw ListenerException();
  }
  EV_SET(change_event, new_socket, EVFILT_READ, EV_ADD, 0, 0, NULL);
  if (kevent(kq, change_event, 1, NULL, 0, NULL) < 0) {
    perror("kevent error");
  }
  std::cout << std::endl
            << "*** New connection from "
            << inet_ntop(client_address.ss_family, getAddress((struct sockaddr*)&client_address), remoteIP,
                         INET6_ADDRSTRLEN)
            << " ***" << std::endl
            << std::endl;
}

void Server::sendingMessageBackToClient(int event_fd, HttpResponse const& response) {
  char buffer[BUFSIZE_CLIENT_REQUEST];
  // if (fcntl(event_fd, F_SETFL, O_NONBLOCK) == -1) {
  //   std::cerr << "Cannot set new socket to non blocking." << std ::endl;
  //   throw ListenerException();
  // }
  long int ret_recv = read(event_fd, buffer, BUFSIZE_CLIENT_REQUEST);

  if (ret_recv == 0) {
    std::cerr << std::endl << "Error: Connection closed by client" << std::endl;
    return;
  }
  if (ret_recv < 0) {
    std::cerr << std::endl << "Error: No byte to read" << std::endl;
    return;
  }
  buffer[ret_recv] = '\0';
  std::cout << "Received from client : " << std ::endl << std::endl << buffer << std::endl;
  std::string raw_response = response.getRaw();
  std::cout << "Response sent to client : " << raw_response << std::endl << std::endl;
  send(event_fd, (raw_response + "ok").c_str(), raw_response.length() + 2, 0);
  std::cout << std::endl << "*** Message sent to client ***" << std::endl;
}

void* Server::run() {
  try {
    createListenerSocket();
    startListening();
    kq = kqueue();
    struct timespec timeout = {0, 0};  // pas de timeout => ne bloque pas
    EV_SET(change_event, listener, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);
    if (kevent(kq, change_event, 1, NULL, 0, &timeout) == -1) {
      perror("kevent");
      throw ServerCoreFatalException();
    }
    while (true) {
      try {
        new_events = kevent(kq, NULL, 0, event, 1, &timeout);
        if (new_events == -1) {
          std::cout << "kevent error" << std::endl;
          throw ServerCoreNonFatalException();
        }
        for (int i = 0; i < new_events; i++) {
          int event_fd = event[i].ident;
          if ((event[i].flags & EV_EOF) != 0) {
            printf("Client has disconnected");
            close(event_fd);
          } else if (event_fd == listener) {
            getClientRequest(event_fd);
          } else if ((event[i].filter & EVFILT_READ) != 0) {
            HttpResponse resp(HttpResponseSuccess::_200, "<html><body><h1>hello</h1></body></html>", "text/html",
                              config_);
            sendingMessageBackToClient(event_fd, resp);
            close(event_fd);
          }
        }
      } catch (ServerCoreNonFatalException& e) {
        std::cerr << e.what() << std::endl;
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
