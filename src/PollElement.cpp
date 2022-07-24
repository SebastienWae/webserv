#include "PollElement.h"

PollElement::PollElement() {
  poll_fd_size = 0;
  clients_array = NULL;
  active_fds = 0;
};

PollElement::~PollElement() {
  if (clients_array != NULL) {
    delete[] clients_array;
  }
};

PollElement PollElement::addToPollfds(int new_socket) {
  if (poll_fd_size == active_fds) {
    Client* new_client_array = new Client[poll_fd_size * 2];
    for (int i = 0; i < poll_fd_size; i++) {
      new_client_array[i] = clients_array[i];
    }
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

std::ostream& operator<<(std::ostream& o, PollElement const& poll_elem) {
  std::cout << "--------------------------------------------------------" << std::endl
            << "Client Array :" << std::endl;
  for (int i = 0; i < poll_elem.poll_fd_size; i++) {
    std::cout << "----------" << std::endl;
    std::cout << "Index " << i << ": " << std::endl;
    std::cout << "Client IP: " << poll_elem.clients_array[i].remoteIP
              << "Fd: " << poll_elem.clients_array[i].fd_info.fd
              << " - Event: " << poll_elem.clients_array[i].fd_info.events
              << " - Revent: " << poll_elem.clients_array[i].fd_info.revents << std::endl;
    std::cout << "Request: " << poll_elem.clients_array[i].request << std::endl;
    std::cout << "Response: " << poll_elem.clients_array[i].response << std::endl;
  }
  std::cout << "--------------------------------------------------------" << std::endl;
  return o;
}

// TO DO
// void PollElement::deleteFromPollfds(int index) {

// };
