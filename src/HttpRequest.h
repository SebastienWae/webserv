#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "Http.h"
#include "Uri.h"

class HttpRequest {
public:
  enum status {
    S_NONE,
    S_OK,
    S_CONTINUE,
    S_BAD_REQUEST,
    S_NOT_IMPLEMENTED,
    S_HTTP_VERSION_NOT_SUPPORTED,
    S_EXPECTATION_FAILED,
    // S_PAYLOAD_TOO_LARGE  // TODO: ????
  };

  typedef std::map<std::string, enum Http::method> MethodMap;
  static const MethodMap method_map;

  HttpRequest(std::string const& raw);

  bool addChunk(std::string const& chunk);

  ~HttpRequest();

  enum status getStatus() const;
  enum Http::method getMethod() const;
  Uri const& getUri() const;
  std::map<std::string, std::string> const& getHeaders() const;
  std::string const& getBody() const;
  std::string getHost() const;

private:
  enum req_parse_state {
    S_REQ_METHOD,
    S_REQ_URI,
    S_REQ_VERSION,
    S_REQ_HEADER_NAME,
    S_REQ_HEADER_SEP,
    S_REQ_HEADER_VAL,
    S_REQ_CRLF,
  };
  enum chunk_parse_state { S_CHK_IDENTIFY, S_CHK_SIZE, S_CHK_EXT, S_CHK_DATA, S_CHK_LAST, S_CHK_TRAILER, S_CHK_CRLF };

  enum status status_;
  enum Http::method method_;
  Uri uri_;
  std::map<std::string, std::string> headers_;
  std::string body_;
};

#endif
