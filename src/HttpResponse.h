#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

#include "HttpResponseStatus.h"
#include "ServerConfig.h"

class HttpResponse {
public:
  HttpResponse(HttpResponseInfo::code status_code, ServerConfig const& server_config);
  HttpResponse(HttpResponseSuccess::code status_code, ServerConfig const& server_config);
  HttpResponse(HttpResponseSuccess::code status_code, std::string const& body, std::string const& content_type,
               ServerConfig const& server_config);
  HttpResponse(HttpResponseRedir::code status_code, ServerConfig const& server_config);
  HttpResponse(HttpResponseClientError::code status_code, ServerConfig const& server_config);
  HttpResponse(HttpResponseServerError::code status_code, ServerConfig const& server_config);

  ~HttpResponse();

  std::string getRaw() const;

private:
  ServerConfig const server_config_;
  std::string const status_code_;
  std::string const reason_phrase_;
  std::string const content_type_;
  std::string body_;
  std::map<std::string, std::string> headers_;
};

#endif
