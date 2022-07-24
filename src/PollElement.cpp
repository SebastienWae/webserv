#include "PollElement.h"

#include <unistd.h>

Client::Client() {}

Client::~Client() {}

PollElement::PollElement(){};

PollElement::~PollElement(){};

PollElement PollElement::addToPollfds(int new_socket) {
  if (poll_fd_size == active_fds) {
    Client* new_client_array = new Client[poll_fd_size * 2];
    for (int i = 0; i < poll_fd_size; i++) new_client_array[i] = clients_array[i];
    poll_fd_size *= 2;
    delete (this->clients_array);
    clients_array = new_client_array;
  }
  clients_array[active_fds].fd_info.fd = new_socket;
  clients_array[active_fds].fd_info.events = POLLIN;
  active_fds++;
  return (*this);
}

void PollElement::initPollElement(int listener) {
  poll_fd_size = POLL_INIT_NUMBER;
  clients_array = new Client[POLL_INIT_NUMBER];
  clients_array[0].fd_info.fd = listener;    // 1st fd for listener
  clients_array[0].fd_info.events = POLLIN;  // listener waiting for POLLIN event
  active_fds = 1;
};

// TO DO
// void PollElement::deleteFromPollfds(int index) {

// };
