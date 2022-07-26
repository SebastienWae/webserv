#include "OnePort.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <exception>

OnePort ::OnePort(){	};

OnePort::~OnePort() { delete[] poll_elem.poll_fds; };

char const* OnePort::ServerCoreFatalException::what() const throw() {
  return "Server core fatal error.";
}
char const* OnePort::ServerCoreNonFatalException::what() const throw() { return "Server error."; }
char const* OnePort::ListenerException::what() const throw() { return "Listener error."; }
char const* OnePort::PollException::what() const throw() { return "Poll error."; }
char const* OnePort::ClientGetRequestException::what() const throw() {
  return "Error while getting client request.";
}
char const* OnePort::ClientSendResponseException::what() const throw() {
  return "Error while sending response to client.";
}

void* OnePort::getAddress(struct sockaddr* sockaddress) {
  if (sockaddress->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sockaddress)->sin_addr);
  }
  return &(((struct sockaddr_in6*)sockaddress)->sin6_addr);
}

// If try to read from non-blocking socket without data => not allowed to
// block—it => return -1 + errno set to EAGAIN or EWOULDBLOCK
void OnePort::startListening() const {
  if (listen(listener, MAX_PENDING_CONNECTIONS) < 0) {
    std::cerr << "Listen failed" << std ::endl;
    throw ListenerException();
  }
  if (fcntl(listener, F_SETFL, O_NONBLOCK) == -1) {
    std::cerr << "Cannot set fd to non blocking." << std ::endl;
    throw ListenerException();
  }
}

void OnePort::createListenerSocket() {
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

  if (getaddrinfo(NULL, (this->port).c_str(), &hints, &address_info) != 0) {
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

void OnePort::pollProcessInit() {
  poll_elem.poll_ret = poll(&poll_elem.poll_fds[0], poll_elem.active_fds, 5000);
  // 5000 :  timeout / pas de timeout => set a -1
  if (poll_elem.poll_ret == -1) {
    std::cerr << "Poll error." << std ::endl;
    throw PollException();
  }
  if (poll_elem.poll_ret == 0) {
    std::cout << "Poll timed out" << std ::endl;
    // close connection with client when timeout ?
  }
}

/* Called when data is ready to be received */
void OnePort::getClientRequest() {
  address_len = sizeof(client_address);
  if ((new_socket = accept(listener, (struct sockaddr*)&client_address, (socklen_t*)&address_len))
      < 0) {
    std::cerr << "Accept error" << std ::endl;
    throw ClientGetRequestException();
  }
  poll_elem = poll_elem.addToPollfds(new_socket);
  std::cout << std::endl
            << "*** New connection from "
            << inet_ntop(client_address.ss_family, getAddress((struct sockaddr*)&client_address),
                         remoteIP, INET6_ADDRSTRLEN)
            << " ***" << std::endl
            << std::endl;
}

std::string OnePort::getResponse() const {  // TO DO : put the right thing
  std::string hello
      = "HTTP/1.1 200 OK\nContent-Type : "
        "text/plain\nContent-Length "
        ": 12\n\nHello world !";
  return hello;
}


void OnePort::sendingMessageBackToClient(int i) {
  char buffer[BUFSIZE_CLIENT_REQUEST];  // TO SET
  long int ret_recv = recv(poll_elem.poll_fds[i].fd, buffer, BUFSIZE_CLIENT_REQUEST, 0);

  if (ret_recv == 0) {
    std::cerr << std::endl << "Error: Connection closed by client" << std::endl;
    poll_elem = poll_elem.removeFromPollfds(i);
    throw ClientSendResponseException();
  }
  if (ret_recv < 0) {
    std::cerr << std::endl << "Error: No byte to read" << std::endl;
    poll_elem = poll_elem.removeFromPollfds(i);
    throw ClientSendResponseException();
  }
  std::cout << "Received from client : " << std ::endl << std::endl << buffer << std::endl;

  std::string response = getResponse();  // PUT HERE FUNCTION TO GET THE MESSAGE TO SEND BACK

  send(poll_elem.poll_fds[i].fd, response.c_str(), response.length(), 0);
  std::cout << std::endl << "*** Message sent to client ***" << std::endl;
}

void* OnePort::launchOnOnePort() {
  try {
    createListenerSocket();
    startListening();
    poll_elem.initPollElement(listener);

    std::cout << std::endl
              << "-----------> READY TO START ON PORT " << port << " <-----------" << std::endl
              << std::endl;

    while (true) {
      pollProcessInit();

      for (int i = 0; i < poll_elem.poll_fd_size; i++) {
        try {

          if ((poll_elem.poll_fds[i].revents & POLLIN) != 0) {
            if (poll_elem.poll_fds[i].fd == listener) {
              getClientRequest();
            } else {
              sendingMessageBackToClient(i);
              poll_elem.removeFromPollfds(i);
              close(new_socket);
              std::cout << poll_elem << std::endl;
            }
          }
        } catch (ServerCoreNonFatalException& e) {
          std::cerr << e.what() << std::endl;
        }
      }
    }
    close(listener);
  } catch (ServerCoreFatalException& e) {
    std::cerr << "FATAL ERROR - SERVER STOPPED LISTENING ON PORT " << port << std::endl;
  }
  return (0);
}

void* OnePort::launchHelper(void* current) { return ((OnePort*)current)->launchOnOnePort(); };