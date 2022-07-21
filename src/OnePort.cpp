#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <poll.h>
#include <pthread.h>
#include <sys/_pthread/_pthread_t.h>
#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <algorithm>
#include <exception>
#include <iostream>
#include <list>

#include "OnePort.h"
// NEXT THING TO DO : exception class
// si chunk : => message qui demande si ok pour envoyer gros truc = > return 100
// et continuer a lire (= ne pas fermer socket) jusque timeout ou jusque check
// by seb = is complete a mettre dans un try catch

OnePort ::OnePort(){};

OnePort::~OnePort(){};

void* OnePort::getAddress(struct sockaddr* sockaddress) {
  if (sockaddress->sa_family == AF_INET)  // IPV4
    return &(((struct sockaddr_in*)sockaddress)->sin_addr);

  return &(((struct sockaddr_in6*)sockaddress)->sin6_addr);  // IPV6
}

// If try to read from non-blocking socket without data => not allowed to
// block—it => return -1 + errno set to EAGAIN or EWOULDBLOCK
void OnePort::startListening() const {
  if (listen(listener, MAX_PENDING_CONNECTIONS) < 0) {
    std::cerr << "Listen failed" << std ::endl;
    throw std::exception();
  }
  /* Sets to non blocking mode */
  if (fcntl(listener, F_SETFL, O_NONBLOCK) == -1) {
    std::cout << "Cannot set fd to non blocking." << std ::endl;
    throw std::exception();
  }
}

void OnePort::createListenerSocket() {
  int y = 1;
  memset((char*)&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, (this->port).c_str(), &hints, &address_info) != 0) {
    std::cout << "Getaddrinfo error." << std ::endl;
    throw std::exception();
  }

  /* 1. get listener socket */
  for (p = address_info; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_flags);
    if (listener < 0) {
      continue;
    }
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));

    /* 2. bind listener socket */
    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener);
      continue;
    }
    break;  // a listener is available
  }
  freeaddrinfo(address_info);
  if (p == NULL) {
    std::cout << "Bind failed" << std ::endl;
    throw std::exception();
  }
};

void OnePort::pollProcessInit() {
  poll_elem.poll_ret = poll(&poll_elem.poll_fds[0], poll_elem.active_fds, 5000);
  // 5000 :  timeout / pas de timeout => set a -1
  if (poll_elem.poll_ret == -1) {
    std::cout << "Poll error." << std ::endl;
    throw std::exception();
  }
  if (poll_elem.poll_ret == 0) {  // pas d'event
    std::cout << "Poll timed out" << std ::endl;
  }
}

/* Called when data is ready to be received */
void OnePort::getClientRequest() {
  address_len = sizeof(client_address);
  if ((new_socket = accept(listener, (struct sockaddr*)&client_address,
                           (socklen_t*)&address_len)) < 0) {
    std::cerr << "Accept error" << std ::endl;
  } else {
    poll_elem = poll_elem.addToPollfds(new_socket);
    std::cout << std::endl
              << "*** New connection from "
              << inet_ntop(client_address.ss_family,
                           getAddress((struct sockaddr*)&client_address),
                           remoteIP, INET6_ADDRSTRLEN)
              << " ***" << std::endl
              << std::endl;
  }
}

void OnePort::sendingMessageBackToClient(int i) const {
  char buffer[BUFSIZE_CLIENT_REQUEST];  // TO SET
  long int ret_recv =
      recv(poll_elem.poll_fds[i].fd, buffer, BUFSIZE_CLIENT_REQUEST, 0);

  if (ret_recv == 0) {
    std::cerr << std::endl << "Error: Connection closed by client" << std::endl;
    // TO DO : delete ?
  } else if (ret_recv < 0) {
    std::cerr << std::endl << "Error: No byte to read" << std::endl;
    // TO DO : delete ?
  } else {
    std::cout << "Received from client : " << std ::endl
              << std::endl
              << buffer << std::endl;
    // PUT HERE FUNCTION TO GET THE MESSAGE TO SEND BACK
    std::string hello =
        "HTTP/1.1 200 OK\nContent-Type : "
        "text/plain\nContent-Length "
        ": 12\n\nHello world !";
    send(poll_elem.poll_fds[i].fd, hello.c_str(), hello.length(), 0);
    std::cout << std::endl << "*** Message sent to client ***" << std::endl;

    std::cout << std::endl
              << "Active fds : " << poll_elem.active_fds
              << " - TO DO : CHECK THIS" << std::endl
              << std::endl
              << "---------------------------------------" << std::endl
              << std::endl;
    //  TO DO : verif what happens with fd_elem. fd counts increases
    //  undefined behavior when close and awful crash when deleting
  }
}

void* OnePort::launchOnOnePort() {
  try {
    createListenerSocket();
    startListening();
    poll_elem.initPollElement(listener);

    std::cout << std::endl
              << "-----------> READY TO START ON PORT " << port
              << " <-----------" << std::endl
              << std::endl;

    while (true) {
      pollProcessInit();
      for (int i = 0; i < poll_elem.active_fds; i++) {
        if ((poll_elem.poll_fds[i].revents & POLLIN) != 0) {
          if (poll_elem.poll_fds[i].fd == listener) {
            getClientRequest();
          } else {
            sendingMessageBackToClient(i);
            close(new_socket);
          }
        }
      }
    }
    close(listener);
  } catch (std::exception& e) {
    std::cerr << "*** ERROR - OnePort STOPPED LISTENING ON PORT" << port
              << std::endl;
    // TO DO : close / delete
  }
  return (0);
}

void* OnePort::launchHelper(void* current) {
  return ((OnePort*)current)->launchOnOnePort();
};