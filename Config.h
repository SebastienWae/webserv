#ifndef CONFIG_HPP
#define CONFIG_HPP
#include <__nullptr>
#include <iostream>

// struct location {
//   std::string deny;
//   std::string access_log;
//   std::string log_not_found;

//   location() : deny(nullptr), access_log(nullptr), log_not_found(nullptr) {}
//   ~location();
// };

// struct data {
//   int port;
//   std::string server_names;
//   location urls;
//   location dotfiles;
//   std::string error_log;
//   std::string access_log;
//   std::string error_page;

//   struct data* next;

//   data() : port(0), server_names(nullptr), next(nullptr) {}
//   ~data();
// };

class config {
public:
  config();
  config& operator=(config const& rhs);
  config(config const& src);
  ~config();
  void checkconfig(const std::string& files);
  void setconfig(const std::string& files);

private:
  int port;
  std::string server_names;
};

#endif
