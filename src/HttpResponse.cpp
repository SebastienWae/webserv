#include "HttpResponse.h"

#include <__nullptr>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <ios>
#include <limits>
#include <map>
#include <string>
#include <utility>

#include "File.h"
#include "Http.h"
#include "HttpResponseStatus.h"
#include "ServerConfig.h"

HttpResponse::HttpResponse(HttpResponseInfo::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseInfo::status.at(status_code).first),
      reason_phrase_(HttpResponseInfo::status.at(status_code).second),
      file_(nullptr) {
  headers_["connection"] = "close";
  headers_["content-length"] = "0";
}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second),
      file_(nullptr) {
  headers_["connection"] = "close";
  headers_["content-length"] = "0";
}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code, std::string const& body,
                           std::string const& content_type, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second),
      body_(body),
      file_(nullptr) {
  headers_["content-type"] = content_type;
  headers_["content-length"] = getContentLenght();
}

HttpResponse::HttpResponse(HttpResponseSuccess::code status_code, File* file, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseSuccess::status.at(status_code).first),
      reason_phrase_(HttpResponseSuccess::status.at(status_code).second),
      file_(file) {
  headers_["content-type"] = file_->getMimeType();
  headers_["content-length"] = getContentLenght();
}

HttpResponse::HttpResponse(HttpResponseRedir::code status_code, std::string const& location)
    : status_code_(HttpResponseRedir::status.at(status_code).first),
      reason_phrase_(HttpResponseRedir::status.at(status_code).second),
      file_(nullptr) {
  headers_["location"] = location;
  headers_["connection"] = "close";
  headers_["content-length"] = "0";
}

HttpResponse::HttpResponse(HttpResponseClientError::code status_code, ServerConfig const* server_config)
    : server_config_(server_config),
      status_code_(HttpResponseClientError::status.at(status_code).first),
      reason_phrase_(HttpResponseClientError::status.at(status_code).second),
      file_(nullptr) {
  if (server_config_ != NULL) {
    File* error_page = server_config_->getErrorPage(status_code);
    if (error_page != NULL) {
      body_ = error_page->getContent();
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
      reason_phrase_(HttpResponseServerError::status.at(status_code).second),
      file_(nullptr) {
  if (server_config_ != NULL) {
    File* error_page = server_config_->getErrorPage(status_code);
    if (error_page != NULL) {
      body_ = error_page->getContent();
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
HttpResponse::~HttpResponse() {
  if (file_ != nullptr) {
    delete file_;
  }
}

std::string HttpResponse::getContentLenght() const {
  std::size_t size;
  if (file_ != nullptr) {
    size = file_->getSize();
  } else {
    size = body_.size();
  }
  std::ostringstream len;
  len << size;
  return len.str();
}

std::string HttpResponse::getHeaders() const {
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

  return result;
}

char* HttpResponse::getContent(std::size_t len) {
  char* response = reinterpret_cast<char*>(std::calloc(len + 1, sizeof(char)));
  if (response != nullptr) {
    if (file_ != nullptr) {
      len = len == 0 ? 1 : len;
      if (file_->getIStream()->good() || !file_->getIStream()->eof()) {
        file_->getIStream()->read(response, len);  // NOLINT
        response[len] = 0;
      } else {
        throw EndOfResponseException();
      }
    } else {
      if (body_.empty()) {
        throw EndOfResponseException();
      }
      if (len == 0 || len >= body_.size()) {
        std::memmove(response, body_.c_str(), body_.size());
        body_.erase();
      } else {
        std::memmove(response, body_.c_str(), len);
        body_ = body_.substr(len + 1);
      }
    }
  }
  return response;
}