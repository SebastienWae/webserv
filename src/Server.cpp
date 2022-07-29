#include "Server.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/_types/_uintptr_t.h>
#include <sys/event.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <exception>
#include <iterator>

#include "HttpResponse.h"
#include "HttpResponseStatus.h"
#include "ServerConfig.h"

Server::Server(Config& servers_list)
    : servers_list_(servers_list){

    };

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
    std::cerr << strerror(errno);
    std::cerr << "Bind failed" << std ::endl;
    throw ListenerException();
  }
  if (listen(new_listener, MAX_PENDING_CONNECTIONS) < 0) {
    std::cerr << "Listen failed" << std ::endl;
    throw ListenerException();
  }
  listeners.push_back(new_listener);
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

// void Server::sendingMessageBackToClient(int event_fd, HttpResponse const& response) {
//   char buffer[BUFSIZE_CLIENT_REQUEST];
//   // if (fcntl(event_fd, F_SETFL, O_NONBLOCK) == -1) {
//   //   std::cerr << "Cannot set new socket to non blocking." << std ::endl;
//   //   throw ListenerException();
//   // }
//   long int ret_recv = read(event_fd, buffer, BUFSIZE_CLIENT_REQUEST);

//   if (ret_recv == 0) {
//     std::cerr << std::endl << "Error: Connection closed by client" << std::endl;
//     return;
//   }
//   if (ret_recv < 0) {
//     std::cerr << std::endl << "Error: No byte to read" << std::endl;
//     return;
//   }
//   buffer[ret_recv] = '\0';
//   std::cout << "Received from client : " << std ::endl << std::endl << buffer << std::endl;
//   std::string raw_response = response.getRaw();
//   std::cout << "Response sent to client : " << raw_response << std::endl << std::endl;
//   send(event_fd, (raw_response + "ok").c_str(), raw_response.length() + 2, 0);
//   std::cout << std::endl << "*** Message sent to client ***" << std::endl;
// }

void Server::sendingMessageBackToClient(int event_fd) {
  char buffer[BUFSIZE_CLIENT_REQUEST];

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
  std::string resp = "blibli";
  HttpResponse r(HttpResponseSuccess::_200, NULL);
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
        std::cout << "Opening Port " << it->getport() << std::endl;
      } catch (ServerCoreNonFatalException& e) {
        std::cout << "Cannot listen on port " << it->getport() << std::endl;
      }
    }
    kq = kqueue();
    struct kevent change_event[listeners.size()];
    struct timespec timeout = {0, 0};

    for (std::vector<int>::iterator it = listeners.begin(); it != listeners.end(); ++it) {
      EV_SET(&change_event[std::distance(listeners.begin(), it)], *it, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
             NULL);  // passer udata
    }
    if (kevent(kq, change_event, listeners.size(), NULL, 0, &timeout) == -1) {
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
          // ServerConfig* cc = event[i].udata;
          if ((event[i].flags & EV_EOF) != 0) {
            printf("Client has disconnected");
            close(event_fd);
          } else if (std::find(listeners.begin(), listeners.end(), event_fd) != listeners.end()) {  // is in
            getClientRequest(event_fd);
          } else if ((event[i].filter & EVFILT_READ) != 0) {
            // sendingMessageBackToClient(event_fd, resp);
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
