
#include <exception>

#include "Config.h"

int main() {
  config test;
  try {
    test.checkconfig("nginx.conf");
    test.parse();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return (0);
}