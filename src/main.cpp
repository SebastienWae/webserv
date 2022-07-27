#include <exception>
#include <vector>

#include "Config.h"
#include "Server.h"
#include "ServerConfig.h"

int main(int argc, char **argv) {
  Config config;
  try {
    config.checkconfig(Config::checkextension(argc, argv));
    config.parse();
    // std::vector<ServerConfig> servers = config.getServerConfig();
    // for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
    //   Server server(*it);
    //   server.run();
    // }
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return (0);
}