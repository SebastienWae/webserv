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
#include <cstring>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "Log.h"
#include "ServerConfig.h"

Cgi::Cgi(Client* client, ServerConfig const* server_config, std::string const& method) {
  std::string tmp;
  std::string tmp2;

  tmp.reserve(ENVCOL);
  tmp2.reserve(ENVCOL);
  std::map<std::string, std::string> headers = client->getRequest()->getHeaders();

  std::string auth = "AUTH_TYPE=";
  std::string auth2 = auth + "";
  env.push_back(auth2);
  std::string content_lenght = "CONTENT_LENGTH=";
  std::string content_lenght2 = content_lenght + headers["content-lenght"];
  env.push_back(content_lenght);
  std::string content_type = "CONTENT_TYPE=";
  std::string content_type2 = content_type + headers["content-type"];
  env.push_back(content_type2);
  std::string gate_way = "GATEWAY_INTERFACE=";
  std::string gate_way2 = gate_way + "CGI/1.1";
  env.push_back(gate_way2);
  std::string path_translated = "PATH_TRANSLATED=";
  std::string path_translated2 = path_translated + client->getRequest()->getUri().getDecodedPath();
  env.push_back(path_translated2);
  std::string query_string = "QUERY_STRING=";
  std::string query_string2 = query_string + client->getRequest()->getUri().getQuery();
  env.push_back(query_string2);
  std::string remote_addr = "REMOTE_ADDR=";
  std::string remote_addr2 = remote_addr + "";  // TODO:
  env.push_back(remote_addr2);
  std::string remote_host = "REMOTE_HOST=";
  std::string remote_host2 = remote_host + "";  // vide
  env.push_back(remote_host2);
  std::string remote_ident = "REMOTE_IDENT=";
  std::string remote_ident2 = remote_ident + "";  // vide
  env.push_back(remote_ident2);
  std::string remote_user = "REMOTE_USER=";
  std::string remote_user2 = remote_user + "";  // vide
  env.push_back(remote_user2);
  std::string request_method = "REQUEST_METHOD=";
  std::string request_method2 = request_method + method;
  env.push_back(request_method2);
  std::string scipt_name = "SCRIPT_NAME=";
  std::string scipt_name2 = scipt_name + client->getRequest()->getUri().getPath();
  env.push_back(scipt_name2);
  std::string server_name = "SERVER_NAME=";
  std::string server_name2 = server_name + server_config->getHostname();
  env.push_back(server_name2);
  std::string server_port = "SERVER_PORT=";
  std::string server_port2 = server_port + server_config->getPort();
  env.push_back(server_port2);
  std::string server_protocol = "SERVER_PROTOCOL=";
  std::string server_protocol2 = server_protocol + "HTTP/1.1";
  env.push_back(server_protocol2);
  std::string server_software = "SERVER_SOFTWARE=";
  std::string server_software2 = server_software + "";
  env.push_back(server_software2);
  (void)client;
}
Cgi::Cgi() {}

void Cgi::executeCgi(int const& kq, Client* client) {
  struct kevent kev;
  char* arr[env.size() + 1];
  for (std::vector<std::string>::iterator it = env.begin(); it != env.end(); ++it) {
    arr[std::distance(env.begin(), it)] = const_cast<char*>(it->c_str());
  }
  arr[env.size()] = NULL;
  timespec timeout = {1, 0};
  std::string templat = "/tmp/WebServXXXXXX";
  std::string output;
  pid_t parent;
  int fd = mkstemp(const_cast<char*>(templat.c_str()));
  EV_SET(&kev, fd, EVFILT_READ, EV_ADD | EV_ENABLE, 0, reinterpret_cast<long>(client), &timeout);
  int n = kevent(kq, &kev, 1, NULL, 0, 0);
  if (n == 0) {
    INFO("CGI : Kevent timeout");
  } else if (n < 0) {
    ERROR(std::strerror(errno));
  }

  parent = fork();
  std::string test = "/goinfre/jperras/server/test.sh";
  if (parent == 0) {
    dup2(fd, STDOUT_FILENO);
    execve(test.c_str(), NULL, arr);
    std::cout << std::strerror(errno) << std::endl;
    exit(0);
  }
  close(fd);
}

Cgi::~Cgi() {}