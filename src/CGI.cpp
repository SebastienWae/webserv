#include "CGI.h"

#include <sys/errno.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>

#include "File.h"

CGI::CGI(Client* client, ServerConfig const* server_config, File const* target, std::string const& method)
    : target_(target), client_(client) {
  HttpRequest const* req = client->getRequest();
  Uri const* uri = req->getUri();

  std::map<std::string, std::string> headers = req->getHeaders();

  std::string auth = "AUTH_TYPE=";
  env_.push_back(auth);

  std::string content_lenght = "CONTENT_LENGTH=" + headers["content-lenght"];
  env_.push_back(content_lenght);

  std::string content_type = "CONTENT_TYPE=" + headers["content-type"];
  env_.push_back(content_type);

  std::string gate_way = "GATEWAY_INTERFACE=CGI/1.1";
  env_.push_back(gate_way);

  std::string path_translated
      = "PATH_TRANSLATED=" + server_config->matchRoute(uri)->matchCGI(uri)->getPath() + uri->getDecodedPath();
  env_.push_back(path_translated);

  std::string query_string = "QUERY_STRING=" + uri->getQuery();
  env_.push_back(query_string);

  std::string remote_addr = "REMOTE_ADDR=" + static_cast<std::string>(inet_ntoa(client->getIp()));
  env_.push_back(remote_addr);

  std::string remote_host = "REMOTE_HOST=";
  env_.push_back(remote_host);

  std::string remote_ident = "REMOTE_IDENT=";
  env_.push_back(remote_ident);

  std::string remote_user = "REMOTE_USER=";
  env_.push_back(remote_user);

  std::string request_method = "REQUEST_METHOD=" + method;
  env_.push_back(request_method);

  std::string scipt_name = "SCRIPT_NAME=" + uri->getPath();
  env_.push_back(scipt_name);

  std::string server_name = "SERVER_NAME=" + server_config->getHostname();
  env_.push_back(server_name);

  std::string server_port = "SERVER_PORT=" + server_config->getPort();
  env_.push_back(server_port);

  std::string server_protocol = "SERVER_PROTOCOL=HTTP/1.1";
  env_.push_back(server_protocol);

  std::string server_software = "SERVER_SOFTWARE=";
  env_.push_back(server_software);

  for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
    if (it->first != "authentication" && it->first != "content-lenght" && it->first != "content-type") {
      env_.push_back("HTTP_" + it->first + "=" + it->second);
    }
  }
}

CGI::~CGI() {}

void CGI::execute() {
  char* arr[env_.size() + 1];
  for (std::vector<std::string>::iterator it = env_.begin(); it != env_.end(); ++it) {
    arr[std::distance(env_.begin(), it)] = const_cast<char*>(it->c_str());
  }
  arr[env_.size()] = NULL;

  // TODO: check pipe return
  int pipe_fd[2];
  pipe(pipe_fd);

  pid_t pid;
  pid = fork();
  if (pid == 0) {
    close(pipe_fd[1]);
    dup2(pipe_fd[0], STDIN_FILENO);

    dup2(client_->getSocket(), STDOUT_FILENO);

    execve(target_->getPath().c_str(), NULL, arr);

    close(pipe_fd[0]);
    std::exit(EXIT_FAILURE);
  } else {
    // TODO: handle large content
    close(pipe_fd[0]);

    std::string content = client_->getRequest()->getBody();
    write(pipe_fd[1], content.c_str(), content.size());

    close(pipe_fd[1]);

    client_->setCGIPID(pid);
  }
}