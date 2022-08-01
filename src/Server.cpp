#include "Server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/_types/_uintptr_t.h>
#include <sys/event.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <exception>
#include <iostream>
#include <iterator>

#include "HttpResponse.h"
#include "HttpResponseStatus.h"
#include "ServerConfig.h"

Server::Server(Config& servers_list) : servers_list_(servers_list){};

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

void Server::createListenersSocket(ServerConfig const& server) {
  int y = 1;
  int new_listener;
  struct addrinfo hints;
  struct addrinfo* address_info;
  struct addrinfo* p;

  std::cout << "Launching listener on port " << server.getport() << std::endl;

  memset((char*)&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;

  if (getaddrinfo(NULL, (server.getport()).c_str(), &hints, &address_info) != 0) {
    std::cerr << "Getaddrinfo error." << std ::endl;
    throw ListenerException();
  }
  for (p = address_info; p != NULL; p = p->ai_next) {
    new_listener = socket(p->ai_family, p->ai_socktype, p->ai_flags);
    if (new_listener < 0) {
      continue;
    }
    setsockopt(new_listener, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    if (fcntl(new_listener, F_SETFL, O_NONBLOCK) == -1) {
      std::cerr << "Cannot set fd to non blocking." << std ::endl;
      throw ListenerException();
    }
    if (bind(new_listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(new_listener);
      continue;
    }
    break;
  }
  freeaddrinfo(address_info);
  if (p == NULL) {
    std::cerr << "Bind failed" << std ::endl;
    throw ListenerException();
  }
  if (listen(new_listener, MAX_PENDING_CONNECTIONS) < 0) {
    std::cerr << "Listen failed" << std ::endl;
    throw ListenerException();
  }
  listeners.push_back(new_listener);
  configs.push_back(server);
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

const ServerConfig* Server::getCurrentConfig(std::string& current_port) {
  for (std::vector<ServerConfig>::const_iterator it = servers_list_.getServerConfig()->begin();
       it != servers_list_.getServerConfig()->end(); ++it) {
    if (current_port == it->getport()) {
      return (&(*it));
    }
  }
  std::cout << "ERROR : cannot find config for port " << current_port << std::endl;
  return (NULL);
}

std::string Server::getClientPort(char buffer[BUFSIZE_CLIENT_REQUEST]) {
  std::string str_buffer = static_cast<std::string>(buffer);
  std::string::size_type host_begin = str_buffer.find("Host");
  std::string::size_type host_end = str_buffer.find('\n', host_begin);
  std::string host = str_buffer.substr(host_begin, host_end - host_begin);
  size_t begin_port = host.find_last_of(':');
  std::string port = host.substr(begin_port + 1);
  return (port.substr(0, port.length() - 1));
}

void Server::sendingMessageBackToClient(int event_fd) {
  char buffer[BUFSIZE_CLIENT_REQUEST];
  std::string current_port;

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
  current_port = getClientPort(buffer);
  const ServerConfig* cc = getCurrentConfig(current_port);
  HttpResponse r(HttpResponseSuccess::_200, cc);
  std::cout << cc->getport() << std::endl;
  send(event_fd, r.getRaw().c_str(), r.getRaw().size(), 0);
  std::cout << std::endl << "*** Message sent to client ***" << std::endl;
}

void Server::closeListeners() {
  for (std::vector<int>::const_iterator it = listeners.begin(); it != listeners.end(); it++) {
    close(*it);
  }
}

void* Server::run() {
  try {
    for (std::vector<ServerConfig>::const_iterator it = servers_list_.getServerConfig()->begin();
         it != servers_list_.getServerConfig()->end(); ++it) {
      try {
        createListenersSocket(*it);
      } catch (ServerCoreNonFatalException& e) {
        std::cout << "Cannot listen on port " << it->getport() << std::endl;
      }
    }
    kq = kqueue();
    struct kevent change_event[listeners.size()];
    struct timespec timeout = {0, 0};
    for (std::vector<int>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
      EV_SET(&change_event[std::distance(listeners.begin(), it)], *it, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
             &configs[std::distance(listeners.begin(), it)]);
    }
    if (kevent(kq, change_event, listeners.size(), NULL, 0, &timeout) == -1) {  // NOLINT
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
          int event_fd = event[i].ident;  // NOLINT
          if ((event[i].flags & EV_EOF) != 0) {
            printf("Client has disconnected");
            close(event_fd);
          } else if (std::find(listeners.begin(), listeners.end(), event_fd) != listeners.end()) {
            getClientRequest(event_fd);
          } else if ((event[i].filter & EVFILT_READ) != 0) {
            sendingMessageBackToClient(event_fd);
            close(event_fd);
          }
        }
      } catch (ServerCoreNonFatalException& e) {
        std::cerr << e.what() << std::endl;
      }
    }
    closeListeners();
  } catch (ServerCoreFatalException& e) {
    std::cout << "FATAL EXCEPTION" << std::endl;
  }
  return NULL;
}
