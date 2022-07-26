#include "HttpResponse.h"

#include <map>
#include <string>
#include <utility>

#include "Http.h"
#include "HttpResponseStatus.h"

HttpResponse::HttpResponse(HttpResponseInfo::code status_code)
    : status_code_(HttpResponseInfo::status.at(status_code).first),
      reason_phrase_(HttpResponseInfo::status.at(status_code).second) {}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code)
    : status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second) {}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code, std::string const& body,
                           std::string const& content_type)
    : status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second),
      body_(body) {
  headers_["Content-Type"] = content_type;
}

HttpResponse::HttpResponse(HttpResponseRedir::code status_code)
    : status_code_(HttpResponseRedir::status.at(status_code).first),
      reason_phrase_(HttpResponseRedir::status.at(status_code).second) {}

HttpResponse::HttpResponse(HttpResponseClientError::code status_code)
    : status_code_(HttpResponseClientError::status.at(status_code).first),
      reason_phrase_(HttpResponseClientError::status.at(status_code).second) {}

HttpResponse::HttpResponse(HttpResponseServerError::code status_code)
    : status_code_(HttpResponseServerError::status.at(status_code).first),
      reason_phrase_(HttpResponseServerError::status.at(status_code).second) {}

HttpResponse::~HttpResponse() {}

std::string HttpResponse::getRaw() const {
  std::string result;
  result.append(HTTP_VERSION);
  result.push_back(SP);
  result.append(status_code_);
  result.push_back(SP);
  result.append(reason_phrase_);
  result.append(CRLF);
  if (!content_type_.empty()) {
    result.append("Content-Type: ");
    result.append(content_type_);
    result.append(CRLF);
  }
  for (std::map<std::string, std::string>::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
    result.append(it->first);
    result.push_back(':');
    result.push_back(SP);
    result.append(it->second);
    result.append(CRLF);
  }
  result.append(CRLF);
  result.append(body_);
  result.append(CRLF);
  return result;
}