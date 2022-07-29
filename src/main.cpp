
#include <pthread.h>

#include <exception>
#include <vector>

#include "Config.h"
#include "Server.h"
#include "ServerConfig.h"

#if defined(__APPLE__) && defined(__MACH__)
#  include <sys/_pthread/_pthread_t.h>
#endif

int main(int argc, char** argv) {
  Config config;
  Server* s;

  try {
    config.checkconfig(Config::checkextension(argc, argv));
    config.parse();

    s = new Server(config);
    s->run();

  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  delete s;
  return (0);
}