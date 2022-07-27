#include "Config.h"

#include <string.h>

#include <__nullptr>
#include <iterator>
#include <vector>

#include "Location.h"
#include "ServerConfig.h"

Config::Config() {
  ser.push_back("server");
  ser.push_back("listen");
  ser.push_back("server_name");
  ser.push_back("port");
  ser.push_back("root");
  ser.push_back("error_page");
  ser.push_back("client_max_body_size");
  ser.push_back("auth");
  loca.push_back("location");
  loca.push_back("name");
  loca.push_back("allow");
  loca.push_back("directory_page");
  loca.push_back("directory_listing");
  loca.push_back("root");
  loca.push_back("upload_store");
  loca.push_back("cgi_pass");
  loca.push_back("return");
}

Config& Config::operator=(Config const& rhs) {
  if (this != &rhs) {
    this->servers = rhs.servers;
  }
  return (*this);
}

Config::Config(Config const& src) { *this = src; }

Config::~Config() {}

std::string Config::checkextension(int argc, char** argv) {
  std::string tmp;
  std::string s;
  std::stringstream ss;
  size_t found;
  if (argc != 2) {
    throw Config::ArgException();
  }
  ss << argv[1];
  ss >> s;
  found = s.find_last_of('.');
  if (found == std::string::npos) {
    throw Config::BadException();
  }
  tmp = s.substr(found, s.size());
  if (tmp.compare(0, strlen(".conf"), ".conf") != 0) {
    throw Config::BadException();
  }
  return (s);
}

std::string Config::delcom(std::string const& str) {
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

std::string Config::openfile(const std::string& files) {
  std::ifstream inputFile;
  std::string str;
  std::stringstream strings;
  inputFile.open(files);
  if (inputFile.fail()) {
    throw Config::FilesException();
  }
  strings << inputFile.rdbuf();
  str = strings.str();
  str = delcom(str);
  return (str);
}

void Config::checkbracket(const std::string& str) {
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
    throw Config::BracketException();
  }
}

void Config::checkconfig(const std::string& files) {
  bool flag = false;
  std::string str;
  int count = 0;
  size_t start = 0;
  size_t end = 0;
  std::string tmp;
  int countserv = -1;
  str = openfile(files);
  if (str.find("server_name ", 0) == std::string::npos && str.find("listen ", 0) == std::string::npos) {
    throw Config::NameException();
  }
  if (str.find("server_name ", 0) != std::string::npos && str.find("listen ", 0) != std::string::npos) {
    throw Config::ConfException();
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

void Config::setserver(std::string const& str, int* countserv) {
  size_t found = 0;
  std::string tmp;
  size_t start = 0;
  size_t end = 0;

  void (ServerConfig::*setserv[7])(const std::string& tmp)
      = {&ServerConfig::setlisten, &ServerConfig::setserver_names, &ServerConfig::setport,
         &ServerConfig::setroot,   &ServerConfig::seterror_page,   &ServerConfig::setclient_max_body_size,
         &ServerConfig::setauth};
  if (str.find("server_name", found) == std::string::npos && str.find("listen", found) == std::string::npos) {
    *countserv = 0;
  } else {
    ServerConfig* sev = new ServerConfig;
    servers.push_back(*sev);
    *countserv = *countserv + 1;
  }
  if (str.find("location", 0) != std::string::npos) {
    for (size_t i = 0; i < str.size(); i++) {
      if (str.compare(i, strlen("location"), "location") == 0) {
        tmp = str.substr(0, i);
        break;
      }
    }
  } else {
    tmp = str;
  }

  for (std::vector<std::string>::iterator it = ser.begin() + 1; it != ser.end(); ++it) {
    start = tmp.find(*it, found);
    if (start != std::string::npos) {
      if (tmp.find(';', end + 1) == std::string::npos) {
        throw Config::CommaException();
      }
      for (size_t k = start; tmp[k] != ';'; k++) {
        end = k;
      }
      end++;
      (this->servers[*countserv].*setserv[std::distance(ser.begin() + 1, it)])(tmp.substr(start, end - start));
    }
  }
  if (str.find("location", 0) != std::string::npos) {
    seeklocation(str.substr(end - start, str.size()), countserv);
  }
}

void Config::seeklocation(std::string const& str, int* countserv) {
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

void Config::setlocation(std::string const& str, const int* countserv) {
  Location* loc = new Location;
  size_t start = 0;
  size_t end = 0;
  size_t found = 0;
  bool flag = false;

  std::vector<void (Location::*)(const std::string& tmp)> setLoc;
  setLoc.push_back(&Location::setname);
  setLoc.push_back(&Location::setallow);
  setLoc.push_back(&Location::setdirectory_page);
  setLoc.push_back(&Location::setdirectory_listing);
  setLoc.push_back(&Location::setroot);
  setLoc.push_back(&Location::setupload_store);
  setLoc.push_back(&Location::setcgi_pass);
  setLoc.push_back(&Location::setredirection);
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

  for (std::vector<std::string>::iterator it = loca.begin() + 2; it != loca.end(); ++it) {
    start = str.find(*it, found);
    if (start != std::string::npos) {
      for (size_t k = start; str[k] != '\n'; k++) {
        end = k;
      }
      (loc->*setLoc[std::distance(loca.begin(), it) - 1])(str.substr(start, end - start + 1));
    }
  }
  this->servers[*countserv].setlocation(*loc);
}

void Config::parse(void) {
  for (size_t j = 0; j < this->servers.size(); j++) {
    this->servers[j].parseserv();
    this->servers[j].checkip();
    this->servers[j].checkport();
    this->servers[j].parserror();
    this->servers[j].splitauth();
    this->servers[j].trimserv();
  }
}

const char* Config::FilesException::what(void) const throw() { return ("Exception  : Fail open file"); }
const char* Config::BracketException::what(void) const throw() { return ("Exception : Bracket expected"); }
const char* Config::NameException::what(void) const throw() { return ("Exception : Need at least one server_name"); }
const char* Config::CommaException::what(void) const throw() { return ("Exception : Comma"); }
const char* Config::ArgException::what(void) const throw() { return ("Exception : Bad Arguments"); }
const char* Config::BadException::what(void) const throw() { return ("Exception : Bad Argument name"); }
const char* Config::ConfException::what(void) const throw() { return ("Exception  : Bad configuration"); }
std::vector<ServerConfig> const* Config::getServerConfig() const { return &servers; }