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
  Loca[3] = "index";
  Loca[4] = "root";
  Loca[5] = "upload_store";
  Loca[6] = "cgi_pass";
  Loca[7] = "return";
}

config& config::operator=(config const& rhs) {
  if (this != &rhs) {
    this->Server = rhs.Server;
  }
  return (*this);
}

config::config(config const& src) { *this = src; }

config::~config() {}
std::string config::checkextension(int argc, char** argv) {
  std::string tmp;
  std::string s;
  std::stringstream ss;
  size_t found;
  if (argc != 2) {
    throw config::ArgException();
  }
  ss << argv[1];
  ss >> s;
  found = s.find_last_of('.');
  if (found == std::string::npos) {
    throw config::BadException();
  }
  tmp = s.substr(found, s.size());
  if (tmp.compare(0, strlen(".conf"), ".conf") != 0) {
    throw config::BadException();
  }
  return (s);
}
std::string config::delcom(std::string const& str) {
  std::string tmp = str;
  size_t start = 0;
  size_t end = 0;
  bool flag = true;
  for (size_t i = 0; i < tmp.size(); i++) {
    if (tmp.compare(i, strlen("#"), "#") == 0) {
      start = i;
      for (size_t j = i; j < tmp.size() && flag; j++) {
        if (tmp.compare(j, strlen("\n"), "\n") == 0) {
          end = j;
          tmp.erase(start, end - start);
          flag = false;
        }
      }
      i = 0;
      flag = true;
    }
  }
  return (tmp);
}

std::string config::openfile(const std::string& files) {
  std::ifstream inputFile;
  std::string str;
  std::stringstream strings;
  inputFile.open(files);
  if (inputFile.fail()) {
    throw config::FilesException();
  }
  strings << inputFile.rdbuf();
  str = strings.str();
  str = delcom(str);
  return (str);
}

void config::checkbracket(const std::string& str) {
  int count = 0;
  for (size_t i = 0; i < str.size(); i++) {
    if (str.compare(i, strlen("{"), "{") == 0) {
      count++;
    }
    if (str.compare(i, strlen("}"), "}") == 0) {
      count--;
    }
  }
  if (count != 0) {
    throw config::BracketException();
  }
}

void config::checkconfig(const std::string& files) {
  bool flag = false;
  std::string str;
  int count = 0;
  size_t start = 0;
  size_t end = 0;
  std::string tmp;
  int countserv = -1;
  str = openfile(files);
  if (str.find("server_name", 0) == std::string::npos) {
    throw config::NameException();
  }
  checkbracket(str);
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

  void (server::*setserv[4])(const std::string& tmp)
      = {&server::setlisten, &server::setserver_names, &server::seterror_page,
         &server::setclient_max_body_size};
  start = str.find("server_name", found);
  if (start == std::string::npos) {
    *countserv = 0;
  } else {
    server* sev = new server;
    this->Server.push_back(*sev);
    *countserv = *countserv + 1;
  }
  if (str.find("location", 0) != std::string::npos) {
    for (size_t i = 0; i < str.size(); i++) {
      if (str.compare(i, strlen("location"), "location") == 0) {
        tmp = str.substr(0, i);
      }
    }
  } else {
    tmp = str;
  }

  for (size_t j = 1; j < 5; j++) {
    start = tmp.find(this->ser[j], found);
    if (start != std::string::npos) {
      if (tmp.find(';', 0) == std::string::npos) {
        throw config::CommaException();
      }
      for (size_t k = start; tmp[k] != ';'; k++) {
        end = k;
      }
      end++;
      (this->Server[*countserv].*setserv[j - 1])(tmp.substr(start, end - start));
    }
  }
  if (str.find("location", 0) != std::string::npos) {
    seeklocation(str.substr(end - start, str.size()), countserv);
  }
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
      if (str.find("location", start) != std::string::npos) {
        end = j;
        setlocation(str.substr(start, end - start + 1), countserv);
        start = end + 1;
      }
    }
  }
}

void config::setlocation(std::string const& str, const int* countserv) {
  Location* loc = new Location;
  size_t start = 0;
  size_t end = 0;
  size_t found = 0;
  bool flag = false;

  void (Location::*setLoc[7])(const std::string& tmp)
      = {&Location::setname,       &Location::setallow,        &Location::setindex,
         &Location::setroot,       &Location::setupload_store, &Location::setcgi_pass,
         &Location::setredirection};
  for (size_t i = 0; i < str.size() && !flag; i++) {
    if (str.compare(i, strlen("location"), "location") == 0) {
      start = i;
      for (size_t j = i; j < str.size() && !flag; j++) {
        if (str.compare(j, 1, "\n") == 0) {
          end = j;
          (loc->*setLoc[0])(str.substr(start, end - start));
          flag = true;
        }
      }
    }
  }

  for (size_t j = 2; j < variable; j++) {
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

void config::parse(void) {
  for (size_t j = 0; j < this->Server.size(); j++) {
    this->Server[j].parseserv();
    this->Server[j].checkip();
    this->Server[j].checkport();
    this->Server[j].trimserv();
  }
}

const char* config::FilesException::what(void) const throw() {
  return ("Exception  : Fail open file");
}
const char* config::BracketException::what(void) const throw() {
  return ("Exception : Bracket expected");
}
const char* config::NameException::what(void) const throw() {
  return ("Exception : Need at least one server_name");
}
const char* config::CommaException::what(void) const throw() { return ("Exception : Comma"); }
const char* config::ArgException::what(void) const throw() { return ("Exception : Bad Arguments"); }
const char* config::BadException::what(void) const throw() {
  return ("Exception : Bad Argument name");
}