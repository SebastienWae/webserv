#include "Utils.h"

#include <string>

std::string Utils::getNextToken(std::string const& s, std::string::const_iterator const& it, char sep) {
  std::size_t dist = std::distance(s.begin(), it);
  std::string::size_type len = s.find(sep, dist);
  return s.substr(dist, len - dist);
}