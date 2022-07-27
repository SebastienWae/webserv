#include "HttpResponse.h"

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "Http.h"
#include "HttpResponseStatus.h"
#include "ServerConfig.h"

HttpResponse::HttpResponse(HttpResponseInfo::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseInfo::status.at(status_code).first),
      reason_phrase_(HttpResponseInfo::status.at(status_code).second) {}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second) {}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code, std::string const& body,
                           std::string const& content_type, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second),
      content_type_(content_type),
      body_(body) {
  (void)server_config_;
  unsigned int x = body_.size();
  std::string str;
  std::ostringstream temp;
  temp << x;
  str = temp.str();
  headers_["Content-length"] = str;
}

HttpResponse::HttpResponse(HttpResponseRedir::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseRedir::status.at(status_code).first),
      reason_phrase_(HttpResponseRedir::status.at(status_code).second) {}

HttpResponse::HttpResponse(HttpResponseClientError::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseClientError::status.at(status_code).first),
      reason_phrase_(HttpResponseClientError::status.at(status_code).second) {
  std::map<enum HttpResponseClientError::code, std::string> client_error_pages = server_config->getclienterror();
  std::map<enum HttpResponseClientError::code, std::string>::const_iterator status_page
      = client_error_pages.find(status_code);
  if (status_page != client_error_pages.end()) {
    std::ifstream status_page_file(status_page->second);
    if (status_page_file.is_open()) {
      content_type_ = "text/html";
      std::stringstream buffer;
      buffer << status_page_file.rdbuf();
      body_ = buffer.str();
      unsigned int x = body_.size();
      std::string str;
      std::ostringstream temp;
      temp << x;
      str = temp.str();
      headers_["Content-length"] = str;
    } else {
      std::cerr << status_page->second << " cannot be opened" << std::endl;
    }
  } else {
    std::cerr << "not found" << std::endl;
  }
}

HttpResponse::HttpResponse(HttpResponseServerError::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseServerError::status.at(status_code).first),
      reason_phrase_(HttpResponseServerError::status.at(status_code).second) {
  std::map<enum HttpResponseServerError::code, std::string> server_error_pages = server_config->getservererror();
  std::map<enum HttpResponseServerError::code, std::string>::const_iterator status_page
      = server_error_pages.find(status_code);
  if (status_page != server_error_pages.end()) {
    std::ifstream status_page_file(status_page->second);
    if (status_page_file.is_open()) {
      content_type_ = "text/html";
      std::stringstream buffer;
      buffer << status_page_file.rdbuf();
      body_ = buffer.str();
      headers_["Connection"] = "close";
      unsigned int x = body_.size();
      std::string str;
      std::ostringstream temp;
      temp << x;
      str = temp.str();
      headers_["Content-length"] = str;
    } else {
      std::cerr << status_page->second << " cannot be opened" << std::endl;
    }
  } else {
    std::cerr << "not found" << std::endl;
  }
}
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