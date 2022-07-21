#include "Config.h"

#include <__nullptr>

config::config() : dt(new data) {}
config& config::operator=(config const& rhs) {
  if (this != &rhs) {
    this->dt = rhs.dt;
  }
  return (*this);
}
config::config(config const& src) { *this = src; }
config::~config() {}
void config::checkconfig(const std::string& files) { (void)files; }
void config::setconfig(const std::string& files) { (void)files; }