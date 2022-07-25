#ifndef POLL_ELEMENT_H
#define POLL_ELEMENT_H

#define POLL_INIT_NUMBER 5

#include <poll.h>

#include <iostream>

class PollElement {
public:
  PollElement();
  ~PollElement();

  struct pollfd* poll_fds;

  int poll_fd_size;  // max size in the array
  int active_fds;    // nb of fd currently used
  int poll_ret;      //  nb of fd in the array that have had an event occur

  void initPollElement(int listener);
  PollElement addToPollfds(int new_socket);
  // TO DO : void deleteFromPollfds(int index);
};
std::ostream& operator<<(std::ostream& o, PollElement const& poll_elem);

#endif