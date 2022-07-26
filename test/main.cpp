
#include <exception>

#include "Config.h"

int main(int argc, char **argv) {
  config test;
  try {
    test.checkconfig(test.checkextension(argc, argv));
    test.parse();
  } catch (std::exception &e) {
    std::cerr << e.what() << std::endl;
  }
  return (0);
}