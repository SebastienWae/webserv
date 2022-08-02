#include "Cgi.h"

#include <err.h>
#include <fcntl.h>
#include <malloc/_malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_types/_size_t.h>
#include <sys/event.h>
#include <unistd.h>

#include <__nullptr>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <string>

#include "Log.h"
#include "ServerConfig.h"

Cgi::Cgi(Client* client, ServerConfig const* server_config, std::string const& method) {
  env["AUTH_TYPE="] = "";
  env["CONTENT_LENGTH="] = "";
  env["CONTENT_TYPE="] = "";
  env["GATEWAY_INTERFACE="] = "CGI/1.1";
  env["PATH_TRANSLATED="] = "";
  env["QUERY_STRING="] = client->getRequest()->getUri().getQuery();
  env["REMOTE_ADDR="] = server_config->getHostname();
  env["REMOTE_HOST="] = "";
  env["REMOTE_IDENT="] = "";
  env["REMOTE_USER="] = "";
  env["REQUEST_METHOD="] = method;
  env["SCRIPT_NAME="] = client->getRequest()->getUri().getPath();
  env["SERVER_NAME="] = server_config->getHostname();
  env["SERVER_PORT="] = server_config->getPort();
  env["SERVER_PROTOCOL="] = "HTTP/1.1";
  env["SERVER_SOFTWARE="] = "";
  (void)client;
}
Cgi::Cgi() {}

void Cgi::executeCgi(int const& kq, Client* client) {
  struct kevent kev;
  timespec timeout = {1, 0};
  std::string templat = "WebServ";
  char** tmp;
  char* const* array = NULL;
  std::string output;
  pid_t parent;
  int fd = mkstemp(const_cast<char*>(templat.c_str()));
  dup2(fd, STDOUT_FILENO);
  EV_SET(&kev, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, reinterpret_cast<long>(client), &timeout);
  int n = kevent(kq, &kev, 1, NULL, 0, 0);
  if (n == 0) {
    INFO("CGI : Kevent timeout");
  } else if (n < 0) {
    ERROR(std::strerror(errno));
  }
  tmp = malloc(sizeof(char) * (env.size() + 1));
  for (std::map<std::string, std::string>::iterator it = env.begin(); it != env.end(); it++) {
    output.append(it->first.c_str(), it->second.c_str());
    tmp[std::distance(it, env.end())] = malloc(sizeof(char) * (output.length() + 1));
    strcpy(tmp[std::distance(it, env.end())], const_cast<char*>(output.c_str()));
  }
  strcpy(array[0], const_cast<char*>(env.find("SERVER_NAME=")->second.c_str()));
  parent = fork();
  if (parent == 0) {
    execve(array[0], (array), tmp);
  }
}

Cgi::~Cgi() {}