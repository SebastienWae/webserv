#include "CGI.h"

#include <_ctype.h>
#include <_types/_uint8_t.h>
#include <sys/errno.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

#include "File.h"

CGI::CGI(Client* client, ServerConfig const* server_config, std::pair<std::string, File*> const& cgi_pair,
         std::string const& method)
    : target_(cgi_pair.second), client_(client) {
  HttpRequest const* req = client->getRequest();
  Uri const* uri = req->getUri();

  std::string::size_type ext = target_->getPath().find(cgi_pair.first) + cgi_pair.first.size();
  std::string script_dir = target_->getPath().substr(0, ext);
  std::string::size_type sep = script_dir.find_last_of('/');
  cwd_ = script_dir.substr(0, sep);

  std::map<std::string, std::string> headers = req->getHeaders();

  std::string auth = "AUTH_TYPE=";
  env_.push_back(auth);

  std::string content_lenght = "CONTENT_LENGTH=" + headers["content-lenght"];
  env_.push_back(content_lenght);

  std::string content_type = "CONTENT_TYPE=" + headers["content-type"];
  env_.push_back(content_type);

  std::string gate_way = "GATEWAY_INTERFACE=CGI/1.1";
  env_.push_back(gate_way);

  ext = uri->getDecodedPath().find(cgi_pair.first);
  ext += cgi_pair.first.size();
  std::string info = uri->getDecodedPath().substr(ext);
  std::string path_info = "PATH_INFO=" + info;
  env_.push_back(path_info);

  std::string path_translated = "PATH_TRANSLATED=" + script_dir;
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
      std::string name = it->first;
      std::transform(name.begin(), name.end(), name.begin(), ::toupper);
      env_.push_back("HTTP_" + name + "=" + it->second);
    }
  }
}

CGI::~CGI() {}

CGI::CGIException::CGIException(std::string const& msg) throw() : msg_(msg) {}
CGI::CGIException::~CGIException() throw() {}
char const* CGI::CGIException::what() const throw() { return msg_.c_str(); }

void CGI::execute() {
  try {
    char* arr[env_.size() + 1];
    for (std::vector<std::string>::iterator it = env_.begin(); it != env_.end(); ++it) {
      arr[std::distance(env_.begin(), it)] = const_cast<char*>(it->c_str());
    }
    arr[env_.size()] = NULL;

    int pipe_fd[2];
    if (pipe(pipe_fd) < 0) {
      throw CGIException("CGI: Pipe failed");
    }

    pid_t pid;
    pid = fork();
    if (pid < 0) {
      close(pipe_fd[1]);
      close(pipe_fd[0]);
      throw CGIException("CGI: Fork failed");
    }
    if (pid == 0) {
      if (chdir(cwd_.c_str()) < 0) {
        close(pipe_fd[1]);
        close(pipe_fd[0]);
        throw CGIException("CGI: Chdir failed");
      }
      close(pipe_fd[1]);
      dup2(pipe_fd[0], STDIN_FILENO);
      dup2(client_->getSocket(), STDOUT_FILENO);
      if (execve(target_->getPath().c_str(), NULL, arr) < 0) {
        close(pipe_fd[0]);
        throw CGIException("CGI: Execve failed");
      };
      close(pipe_fd[0]);
      std::exit(EXIT_FAILURE);
    } else {
      close(pipe_fd[0]);

      std::vector<uint8_t> content = client_->getRequest()->getBody();
      fd_set rfds;
      struct timeval tv;
      int retval;

      FD_ZERO(&rfds);
      FD_SET(0, &rfds);
      tv.tv_sec = 1;
      tv.tv_usec = 0;
      retval = select(1, 0, &rfds, 0, &tv);
      if (retval <= 0) {
      }
      if (write(pipe_fd[1], &(content[0]), content.size()) <= 0) {
      }

      close(pipe_fd[1]);

      client_->setCGIPID(pid);
    }
  } catch (CGIException& e) {
  }
}