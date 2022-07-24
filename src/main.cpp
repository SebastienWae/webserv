#include "OnePort.h"

int main() {
  /* Multiple ports => threads */
  // std::list<std::string> ports;
  // std::list<OnePort> multiple_ports;
  // std::list<pthread_t> threads;

  // std::string port1 = "8080";
  // std::string port2 = "9090";
  // ports.push_back(port1);
  // ports.push_back(port2);

  // for (std::list<std::string>::iterator it = ports.begin(); it !=
  // ports.end();
  //      ++it) {
  //   OnePort* S = new OnePort();
  //   S->port = (*it);
  //   multiple_ports.push_front(*S);
  //   std::cout << "Launching on port " << S->port << std::endl;
  //   pthread_t th;
  //   threads.push_front(th);
  //   pthread_create(&th, NULL, &OnePort::launchHelper, S);
  // }

  // for (std::list<pthread_t>::iterator it = threads.begin(); it !=
  // threads.end();
  //      ++it) {
  //   pthread_join(*it, 0);
  // }

  // for (std::list<OnePort>::iterator it = multiple_ports.begin();
  //      it != multiple_ports.end(); ++it) {
  //   delete &it;
  // }

  /* One port => to work */
  OnePort* S = new OnePort();
  S->port = "8080";
  S->launchOnOnePort();
  delete S;
}