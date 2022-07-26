#ifndef HTTPRESPONSE_H
#define HTTPRESPONSE_H

#include <map>
#include <string>

#include "HttpResponseStatus.h"

class HttpResponse {
public:
  HttpResponse(HttpResponseInfo::code status_code);
  HttpResponse(HttpResponseSuccess::code status_code);
  HttpResponse(HttpResponseSuccess::code status_code, std::string const& body, std::string const& content_type);
  HttpResponse(HttpResponseRedir::code status_code);
  HttpResponse(HttpResponseClientError::code status_code);
  HttpResponse(HttpResponseServerError::code status_code);

  ~HttpResponse();

  std::string getRaw() const;

private:
  std::string const status_code_;
  std::string const reason_phrase_;
  std::string const content_type_;
  std::string const body_;
  std::map<std::string, std::string> headers_;
};

#endif
