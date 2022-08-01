#include "Cgi.h"

Cgi::Cgi() {
  env["AUTH_TYPE"] = "";
  env["CONTENT_LENGTH"] = "";
  env["CONTENT_TYPE"] = "";
  env["GATEWAY_INTERFACE"] = "CGI/1.1";
  env["PATH_TRANSLATED"] = "";
  env["QUERY_STRING"] = "";
  env["REMOTE_ADDR"] = "";
  env["REMOTE_HOST"] = "";
  env["REMOTE_IDENT"] = "";
  env["REMOTE_USER"] = "";
  env["REQUEST_METHOD"] = "";
  env["SCRIPT_NAME"] = "";
  env["SERVER_NAME"] = "";
  env["SERVER_PORT"] = "";
  env["SERVER_PROTOCOL"] = "";
  env["SERVER_SOFTWARE"] = "";
}
Cgi::~Cgi() {}