
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
  std::list<pthread_t> threads;
  std::list<Server> servers_list;

  try {
    config.checkconfig(Config::checkextension(argc, argv));
    config.parse();

    std::vector<ServerConfig> const* servers = config.getServerConfig();

    for (std::vector<ServerConfig>::const_iterator it = servers->begin(); it != servers->end(); ++it) {
      Server* server = new Server(&(*it));
      std::cout << std::endl
                << "-----------> LAUNCHING ON PORT " << server->getConfig()->getport() << " <-----------" << std::endl
                << std::endl;

      servers_list.push_front(*server);
      pthread_t th;

      pthread_create(&th, NULL, &threadWrapper, server);
      threads.push_front(th);
    }
  } catch (std::exception& e) {
    std::cerr << e.what() << std::endl;
  }
  for (std::list<pthread_t>::iterator it = threads.begin(); it != threads.end(); ++it) {
    pthread_join(*it, 0);
  }

  for (std::list<Server>::iterator it = servers_list.begin(); it != servers_list.end(); ++it) {
    delete &it;
  }

  return (0);
}