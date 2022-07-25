#include "PollElement.h"

#include <unistd.h>

#include <iostream>

PollElement::PollElement() {
  poll_fd_size = 0;
  active_fds = 0;
};

PollElement::~PollElement(){};

PollElement PollElement::addToPollfds(int new_socket) {
  if (poll_fd_size == active_fds) {
    struct pollfd* new_pollfd = new struct pollfd[poll_fd_size * 2];
    for (int i = 0; i < poll_fd_size; i++) new_pollfd[i] = poll_fds[i];
    poll_fd_size *= 2;
    delete (this->poll_fds);
    poll_fds = new_pollfd;
  }
  poll_fds[active_fds].fd = new_socket;
  poll_fds[active_fds].events = POLLIN;
  active_fds++;
  return (*this);
}

void PollElement::initPollElement(int listener) {
  poll_fd_size = POLL_INIT_NUMBER;
  poll_fds = new struct pollfd[POLL_INIT_NUMBER];
  poll_fds[0].fd = listener;    // 1st fd for listener
  poll_fds[0].events = POLLIN;  // listener waiting for POLLIN event
  active_fds = 1;
};

std::ostream& operator<<(std::ostream& o, PollElement const& poll_elem) {
  std::cout << "Max size : " << poll_elem.poll_fd_size << " Active fds : " << poll_elem.active_fds
            << std::endl;

  for (int i = 0; i < poll_elem.poll_fd_size; i++) {
    std::cout << "----------" << std::endl;
    std::cout << "Index " << i << ": " << std::endl;
    std::cout << "Fd: " << poll_elem.poll_fds[i].fd << "Event: " << poll_elem.poll_fds[i].events
              << "Event: " << poll_elem.poll_fds[i].revents << std::endl;
  }
  std::cout << "--------------------------------------------------------" << std::endl;
  return o;
}