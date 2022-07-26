#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace Utils {
  std::string getNextToken(std::string const& s, std::string::const_iterator const& it, char sep);
}

#endif
