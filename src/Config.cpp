#include "Config.h"

#include <__config>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

#include "File.h"
#include "Log.h"
#include "ServerConfig.h"

Config::Config(std::string const& config_path) : file_(File(config_path)) {
  INFO("Opening config file: " + config_path)

  if (file_.getType() != File::REG || !file_.isReadable() || file_.getIStream() == NULL) {
    throw ParsingException("Cannot open config file: " + config_path);
  }
}

Config::~Config() {
  for (std::vector<ServerConfig*>::iterator it = servers_.begin(); it != servers_.end(); ++it) {
    delete *it;
  }
}

Config::ParsingException::ParsingException() throw() {}
Config::ParsingException::ParsingException(std::string const& msg) throw() : msg_(msg) {}
Config::ParsingException::~ParsingException() throw() {}
char const* Config::ParsingException::what() const throw() { return msg_.c_str(); }

void Config::parse() {  // NOLINT
  std::ifstream* file = file_.getIStream();
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
              if (hostname.empty()) {
                hostname = "localhost";
              }
              port = line.substr(sep + 1, line.size() - sep - 2);
            }
            try {
              Uri uri("http://" + hostname + ":" + port);
              if (uri.getType() == Uri::TYPE_ABSOLUTE && hostname.back() != '/' && uri.getPath() == "/"
                  && uri.getQuery().empty() && uri.getUserInfo().empty()) {
                for (std::vector<ServerConfig*>::iterator it = servers_.begin(); it != servers_.end(); ++it) {
                  if ((*it)->getHostname() == hostname && (*it)->getPort() == port) {
                    current_server_config = *it;
                  }
                }
              } else {
                throw ParsingException();
              }
            } catch (Uri::UriParsingException& s) {
              throw ParsingException();
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
  current_server_config->verify();
  if (servers_.empty()) {
    throw ParsingException("No server defined in the config file");
  }
}

ServerConfig const* Config::matchServerConfig(std::string const& host) const {
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