#ifndef LOG_H
#define LOG_H

#include <iostream>

#define ERROR(msg)                                                                            \
  std::cout << "[ERROR] " << __BASE_FILE__ << ":" << __LINE__ << ":" << __func__ << std::endl \
            << msg << std::endl                                                               \
            << std::endl;

#define INFO(msg)                                                                            \
  std::cout << "[INFO] " << __BASE_FILE__ << ":" << __LINE__ << ":" << __func__ << std::endl \
            << msg << std::endl                                                              \
            << std::endl;

#endif
