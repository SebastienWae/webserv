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
      reason_phrase_(HttpResponseInfo::status.at(status_code).second) {
  headers_["connection"] = "close";
  headers_["content-length"] = "0";
}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second) {
  headers_["connection"] = "close";
  headers_["content-length"] = "0";
}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code, std::string const& body,
                           std::string const& content_type, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second),
      body_(body) {
  headers_["content-type"] = content_type;
  headers_["content-length"] = getContentLenght();
}

HttpResponse::HttpResponse(HttpResponseRedir::code status_code, std::string const& location)
    : status_code_(HttpResponseRedir::status.at(status_code).first),
      reason_phrase_(HttpResponseRedir::status.at(status_code).second) {
  headers_["location"] = location;
  headers_["connection"] = "close";
  headers_["content-length"] = "0";
}

HttpResponse::HttpResponse(HttpResponseClientError::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseClientError::status.at(status_code).first),
      reason_phrase_(HttpResponseClientError::status.at(status_code).second) {
  if (server_config_ != NULL) {
    std::string error_page = server_config_->getErrorPage(status_code);
    if (!error_page.empty()) {
      std::ifstream file(error_page);
      if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        body_ = buffer.str();
        file.close();
      }
    }
  }

  if (body_.empty()) {
    body_ = "<htm><head><title>" + status_code_ + " " + reason_phrase_ + "</title></head><body><center><h1>"
            + status_code_ + " " + reason_phrase_ + "</h1></center></body><html>";
  }

  headers_["content-type"] = "text/html";
  headers_["connection"] = "close";
  headers_["content-length"] = getContentLenght();
}

HttpResponse::HttpResponse(HttpResponseServerError::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseServerError::status.at(status_code).first),
      reason_phrase_(HttpResponseServerError::status.at(status_code).second) {
  if (server_config_ != NULL) {
    std::string error_page = server_config_->getErrorPage(status_code);
    if (!error_page.empty()) {
      std::ifstream file(error_page);
      if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        body_ = buffer.str();
        file.close();
      }
    }
  }

  if (body_.empty()) {
    body_ = "<htm><head><title>" + status_code_ + " " + reason_phrase_ + "</title></head><body><center><h1>"
            + status_code_ + " " + reason_phrase_ + "</h1></center></body><html>";
  }

  headers_["content-type"] = "text/html";
  headers_["connection"] = "close";
  headers_["content-length"] = getContentLenght();
}
HttpResponse::~HttpResponse() {}

std::string HttpResponse::getContentLenght() const {
  std::string::size_type size = body_.size() + 2;
  std::ostringstream len;
  len << size;
  return len.str();
}

std::string HttpResponse::getRaw() const {
  std::string result;

  result.append(HTTP_VERSION);
  result.push_back(SP);
  result.append(status_code_);
  result.push_back(SP);
  result.append(reason_phrase_);
  result.append(CRLF);

  for (std::map<std::string, std::string>::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
    result.append(it->first);
    result.push_back(':');
    result.push_back(SP);
    result.append(it->second);
    result.append(CRLF);
  }

  result.append(CRLF);

  if (!body_.empty()) {
    result.append(body_);
    result.append(CRLF);
  }
  return result;
}