#ifndef POLL_ELEMENT_H
#define POLL_ELEMENT_H

#include "Client.h"

#define POLL_INIT_NUMBER 5

class PollElement {
public:
  PollElement();
  ~PollElement();

  Client* clients_array;

  int poll_fd_size;  // max size in the array
  int active_fds;    // nb of fd currently used
  int poll_ret;      //  nb of fd in the array that have had an event occur

  void initPollElement(int listener);
  PollElement addToPollfds(int new_socket);
  // TO DO : void deleteFromPollfds(int index); undefined behaviour for now
};

std::ostream& operator<<(std::ostream& o, PollElement const& poll_elem);

#endif