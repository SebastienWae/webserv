#include "PollElement.h"
#include <unistd.h>

PollElement::PollElement(){};

PollElement::~PollElement(){};

PollElement PollElement::addToPollfds(int new_socket) {
  if (poll_fd_size == active_fds) {
    struct pollfd* new_pollfd = new struct pollfd[poll_fd_size * 2];
    for (int i = 0; i < poll_fd_size; i++)
      new_pollfd[i] = poll_fds[i];
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

// TO DO
// void PollElement::deleteFromPollfds(int index) {

// };
