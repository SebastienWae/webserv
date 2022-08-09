#include <exception>
#include <iostream>

#include "Config.h"
#include "Server.h"

void run(Config const& config) {
  try {
    Server server(config);
    server.start();
  } catch (std::exception& e) {
    std::cerr << "[CRITICAL ERROR]: restarting..." << std::endl;
    run(config);
  }
}

int main(int argc, char** argv) {
  if (argc == 1 || argc == 2) {
    std::string config_path;
    if (argc == 1) {
      config_path = "./default.conf";
    } else {
      config_path = argv[1];
    }
    try {
      Config config(config_path);
      config.parse();
      signal(SIGPIPE, SIG_IGN);
      run(config);
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  } else {
    std::cout << "usage: " << argv[0] << " [/path/to/config/file.conf]" << std::endl;
  }
}