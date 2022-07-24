#include "Config.h"

#include <string.h>

#include <__nullptr>

#include "Location.h"
#include "Server.h"

config::config() {
  ser[0] = "server";
  ser[1] = "listen";
  ser[2] = "server_name";
  ser[3] = "error_page";
  ser[4] = "client_max_body_size";
  Loca[0] = "location";
  Loca[1] = "name";
  Loca[2] = "allow";
  Loca[3] = "autoindex";
  Loca[4] = "upload_store";
  Loca[5] = "cgi_pass";
  Loca[6] = "return";
}

config& config::operator=(config const& rhs) {
  if (this != &rhs) {
    this->Server = rhs.Server;
  }
  return (*this);
}

config::config(config const& src) { *this = src; }

config::~config() {}

void config::checkconfig(const std::string& files) {
  (void)files;
  bool flag = false;
  std::ifstream inputFile;
  std::string str;
  std::stringstream strings;
  int count = 0;
  size_t start = 0;
  size_t end = 0;
  std::string tmp;
  int countserv = -1;
  inputFile.open("nginx.conf");
  if (inputFile.fail()) {
    throw config::FilesException();
  }
  strings << inputFile.rdbuf();
  str = strings.str();
  for (size_t i = 0; i < str.size(); i++) {
    if (str.compare(i, strlen("server"), "server") == 0) {
      for (size_t j = i; str.compare(j, 1, "{") != 0; j++) {
        end = j;
      }
      start = end + 1;
      count++;
      end += 2;
      while (count > 0 && !flag) {
        if (str.compare(end, 1, "{") == 0) {
          count++;
        }
        if (str.compare(end, 1, "}") == 0) {
          count--;
        }
        end++;
        if (end > str.size()) {
          flag = true;
        }
      }
      tmp = str.substr(start, end - start);
      setserver(tmp, &countserv);
      i = end;
    }
  }
}

void config::setserver(std::string const& str, int* countserv) {
  size_t found = 0;
  std::string tmp;
  size_t start = 0;
  size_t end = 0;

  void (server::*server_fns_array[4])(const std::string& tmp)
      = {&server::setport, &server::setserver_names, &server::seterror_page,
         &server::setclient_max_body_size};
  start = str.find("server_name", found);
  if (start == std::string::npos) {
    *countserv = 0;
  } else {
    server* sev = new server;
    this->Server.push_back(*sev);
    *countserv = *countserv + 1;
  }

  for (size_t i = 0; i < str.size(); i++) {
    if (str.compare(i, strlen("location"), "location") == 0) {
      tmp = str.substr(0, i);
    }
  }
  for (size_t j = 1; j < 5; j++) {
    start = tmp.find(this->ser[j], found);
    if (start != std::string::npos) {
      for (size_t k = start; tmp[k] != ';'; k++) {
        end = k;
      }
      end++;
      (this->Server[*countserv].*server_fns_array[j - 1])(tmp.substr(start, end - start));
    }
  }

  seeklocation(str.substr(end - start, str.size()), countserv);
}

void config::seeklocation(std::string const& str, int* countserv) {
  std::string tmp;
  size_t start = 0;
  size_t end = 0;
  bool flag = true;

  for (size_t i = 0; i < str.size() && flag; i++) {
    if (str.compare(i, strlen("location"), "location") == 0) {
      start = i;
      flag = false;
    }
  }
  for (size_t j = 0; j < str.size(); j++) {
    if (str.compare(j, strlen("}"), "}") == 0) {
      end = j;
      setlocation(str.substr(start, end - start + 1), countserv);
      start = end + 1;
    }
  }
}

void config::setlocation(std::string const& str, int* countserv) {
  Location* loc = new Location;
  size_t start = 0;
  size_t end = 0;
  size_t found = 0;

  void (Location::*setLoc[6])(const std::string& tmp)
      = {&Location::setname,         &Location::setallow,    &Location::setautoindex,
         &Location::setupload_store, &Location::setcgi_pass, &Location::setredirection};
  for (size_t j = 2; j < 7; j++) {
    start = str.find(this->Loca[j], found);
    if (start != std::string::npos) {
      for (size_t k = start; str[k] != '\n'; k++) {
        end = k;
      }
      (loc->*setLoc[j - 1])(str.substr(start, end - start + 1));
    }
  }
  this->Server[*countserv].setlocation(*loc);
}

const char* config::FilesException::what(void) const throw() {
  return ("Exception  : Fail open files");
}

// std::vector<std::string> split;

// size_t found = 0;
// size_t foundserv = 0;
// bool flag = true;
// int count = 0;
// int count2 = 0;

// std::list<std::string>::iterator it = this->keyword.begin();
// std::string buf;

// found = str.find('{', found);
// for (std::string::iterator ite = (str + found); ite != str.end(); it++) {
//   if (*ite == '{') {
//     count2++;
//   }
//   if (*ite == '}') {
//     count2--;
//   }
//   if (count2 == 0) {
//   }
// }

// servername(str);
// foundserv = str.find(*it, foundserv);
// it++;
// while (it != this->keyword.end()) {
//   while (flag) {
//     found = str.find(*it, found);
//     if (found == std::string::npos) {
//       flag = false;
//     } else {
//       this->Server[count].port = 0;
//       found += (*it).length();
//     }
//   }
//   it++;
//   flag = true;
// }
// it = this->keyword.begin();
// foundserv += (*it).length();
// foundserv = str.find(*it, foundserv);