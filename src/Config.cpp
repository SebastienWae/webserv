#include "Config.h"

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

#include "File.h"
#include "Log.h"
#include "ServerConfig.h"

Config::Config(std::string const& config_path) {
  INFO("Opening config file: " + config_path)

  File file(config_path);
  if (file.getType() == File::REG && file.isReadable() && file.getIStream() != NULL) {
    parse(file.getIStream());
  } else {
    throw ParsingException("Cannot open config file: " + config_path);
  }
}

Config::~Config() {
  for (std::vector<ServerConfig*>::iterator it = servers_.begin(); it != servers_.end(); ++it) {
    delete *it;
  }
}

Config::ParsingException::ParsingException(std::string const& msg) throw() : msg_(msg) {}
Config::ParsingException::~ParsingException() throw() {}
char const* Config::ParsingException::what() const throw() { return msg_.c_str(); }

void Config::parse(std::ifstream* file) {  // NOLINT
  enum parse_state state = S_NONE;
  ServerConfig* current_server_config = NULL;
  Route* current_route = NULL;
  for (std::string line; std::getline(*file, line);) {  // NOLINT
    switch (state) {
      case S_NONE: {
        if (line.empty()) {
          continue;
        }
        if (line.front() == '[' && line.back() == ']') {
          if (line.size() <= 2) {
            current_server_config = new ServerConfig("", "");
          } else {
            std::string::size_type sep = line.find(':');
            std::string hostname;
            std::string port;
            if (sep == std::string::npos) {
              hostname = line.substr(1, line.size() - 2);
              port = "80";
            } else {
              hostname = line.substr(1, sep - 1);
              port = line.substr(sep + 1, line.size() - sep - 2);
            }
            checkPort(port);
            checkHostname(hostname);
            for (std::vector<ServerConfig*>::iterator it = servers_.begin(); it != servers_.end(); ++it) {
              if ((*it)->getHostname() == hostname && (*it)->getPort() == port) {
                current_server_config = *it;
              }
            }
            if (current_server_config == NULL) {
              current_server_config = new ServerConfig(hostname, port);
            }
          }
          servers_.push_back(current_server_config);
          state = S_IN_SERVER;
        } else {
          throw ParsingException("Config file error at line: " + line);
        }
        break;
      }
      case S_IN_SERVER: {
        if (line.empty()) {
          if (current_route != NULL) {
            current_route->verify();
            current_route = NULL;
          }
          if (current_server_config != NULL) {
            current_server_config->verify();
            current_server_config = NULL;
          }
          state = S_NONE;
        } else if (line.size() > 2 && line.substr(0, 2) == "\t\t") {
          if (current_route != NULL) {
            current_route->parse(line.substr(2));
          } else {
            throw ParsingException("Config file error at line: " + line);
          }
        } else if (line.front() == '\t') {
          if (current_route != NULL) {
            current_route->verify();
            current_route = NULL;
          }
          current_route = current_server_config->parse(line.substr(1));
        } else {
          throw ParsingException("Config file error at line: " + line);
        }
        break;
      }
    }
  }
  if (servers_.empty()) {
    throw ParsingException("No server defined in the config file");
  }
}

ServerConfig const* Config::matchServerConfig(HttpRequest const* request) const {
  std::string host = request->getHost();
  std::string hostname;
  std::string port;
  std::string::size_type sep = host.find(':');
  if (sep != std::string::npos) {
    hostname = host.substr(0, sep);
    port = host.substr(sep + 1);
  } else {
    hostname = host;
    port = "80";
  }
  for (std::vector<ServerConfig*>::const_iterator it = servers_.begin(); it != servers_.end(); ++it) {
    if ((*it)->getHostname() == hostname && (*it)->getPort() == port) {
      return *it;
    }
  }
  return servers_[0];
}

std::set<std::string> Config::getPorts() const {
  std::set<std::string> ports;
  for (std::vector<ServerConfig*>::const_iterator it = servers_.begin(); it != servers_.end(); ++it) {
    ports.insert((*it)->getPort());
  }
  return ports;
}

void Config::checkPort(std::string const& port) {  // NOLINT
  if (port.size() > 5) {                           // NOLINT
    throw Config::ParsingException("Too long port");
  }

  if (port.size() == 1) {
    if (port[0] > '9' || port[0] < '0') {
      throw Config::ParsingException("Invalid port");
    }
  } else if (port.size() == 2) {
    if (port[0] > '9' || port[0] < '0' || port[1] > '9' || port[1] < '0') {
      throw Config::ParsingException("Invalid port");
    }
  } else if (port.size() == 3) {
    if (port[0] > '9' || port[0] < '0' || port[1] > '9' || port[1] < '0' || port[2] > '9' || port[2] < '0') {
      throw Config::ParsingException("Invalid port");
    }
  } else if (port.size() == 4) {
    if (port[0] > '9' || port[0] < '0' || port[1] > '9' || port[1] < '0' || port[2] > '9' || port[2] < '0'
        || port[3] > '9' || port[3] < '0') {
      throw Config::ParsingException("Invalid port");
    }
  } else if (port.size() == 5) {  // NOLINT
    if (port[0] > '9' || port[0] < '0' || port[1] > '9' || port[1] < '0' || port[2] > '9' || port[2] < '0'
        || port[3] > '9' || port[3] < '0' || port[4] > '9' || port[4] < '0') {
      throw Config::ParsingException("Invalid port");
    }
    if (port[0] == '6' && port[1] > '5') {
      throw Config::ParsingException("Invalid port");
    }
    if (port[0] == '6' && port[1] == '5' && port[2] > '5') {
      throw Config::ParsingException("Invalid port");
    }
    if (port[0] == '6' && port[1] == '5' && port[2] == '5' && port[3] > '3') {
      throw Config::ParsingException("Invalid port");
    }
    if (port[0] == '6' && port[1] == '5' && port[2] == '5' && port[3] == '3' && port[4] > '5') {
      throw Config::ParsingException("Invalid port");
    }
  }
}

void Config::checkHostname(const std::string& hostname) {  // NOLINT
  std::vector<std::string> tmp;
  size_t start = 0;
  size_t end = 0;
  size_t find = 0;
  size_t count = 0;
  while (find != std::string::npos) {
    find = hostname.find('.', find);
    if (find != std::string::npos) {
      count++;
      find += 1;
    }
  }
  if (count == 3) {
    for (size_t i = 0; i < hostname.size(); i++) {
      end = i;
      if ((hostname.compare(i, strlen("."), ".")) == 0) {
        tmp.push_back(hostname.substr(start, end - start));
        start = end + 1;
      }
    }
    end++;
    tmp.push_back(hostname.substr(start, end - start));
    for (size_t i = 0; i < tmp.size(); i++) {
      for (size_t j = 0; j < tmp[i].size(); j++) {
        if (tmp[i][j] > '9' || tmp[i][j] < '0') {
          throw Config::ParsingException("Invalid IP");
        }
      }
      if (tmp[i][0] > '2' || tmp[i][0] < '0') {
        throw Config::ParsingException("Invalid IP");
      }
      if (tmp[i][0] == '2' && (tmp[i][1] > '5')) {
        throw Config::ParsingException("Invalid IP");
      }
      if (tmp[i][0] == '2' && (tmp[i][1] == '5') && (tmp[i][2] > '5')) {
        throw Config::ParsingException("Invalid IP");
      }
    }
  } else if (!hostname.empty()) {
    {
      std::string::size_type sep = hostname.find('.');
      size_t found = std::string::npos;
      std::string dat = hostname.substr(sep, hostname.size());
      if (dat.compare(0, strlen(".com"), ".com") != 0) {
        throw Config::ParsingException("Invalid IP");
      }
      const std::string FORBIDENCHAR = " \n\r\t\f\v";
      found = hostname.find_first_of(FORBIDENCHAR);
      if (found != std::string::npos) {
        throw Config::ParsingException("Invalid IP");
      }
    }
  }
}