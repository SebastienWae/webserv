#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <cstddef>
#include <map>
#include <string>

#include "HttpResponseStatus.h"
#include "ServerConfig.h"

class HttpResponse {
public:
  class EndOfResponseException : public std::exception {};

  HttpResponse(HttpResponseInfo::code status_code, ServerConfig const* server_config);
  HttpResponse(HttpResponseSuccess::code status_code, ServerConfig const* server_config);
  HttpResponse(HttpResponseSuccess::code status_code, std::string const& body, std::string const& content_type,
               ServerConfig const* server_config);
  HttpResponse(HttpResponseSuccess::code status_code, File* file, ServerConfig const* server_config);
  HttpResponse(HttpResponseRedir::code status_code, std::string const& location);
  HttpResponse(HttpResponseClientError::code status_code, ServerConfig const* server_config);
  HttpResponse(HttpResponseServerError::code status_code, ServerConfig const* server_config);

  ~HttpResponse();

  std::string getHeaders() const;
  std::string getContent(std::size_t len);

private:
  ServerConfig const* server_config_;
  std::string const status_code_;
  std::string const reason_phrase_;
  std::string body_;
  File* file_;
  std::map<std::string, std::string> headers_;

  std::string getContentLenght() const;
};

#endif
