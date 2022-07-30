#include "Config.h"
#include "Log.h"
#include "Server.h"

int main(int argc, char **argv) {
  Config config;
  try {
    config.checkconfig(Config::checkextension(argc, argv));
    config.parse();
    config.parse();
    Server server(config);
    server.start();
  } catch (...) {
    ERROR("Critical error!!");
    main(argc, argv);
  }
}