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
  enum status { S_NONE, S_OK, S_CONTINUE, S_BAD_REQUEST, S_NOT_IMPLEMENTED, S_HTTP_VERSION_NOT_SUPPORTED };

  typedef std::map<std::string, enum Http::method> MethodMap;
  static const MethodMap method_map;

  HttpRequest(std::string const& raw);

  bool addChunk(std::string const& chunk);

  ~HttpRequest();

  enum status getStatus() const;
  enum Http::method getMethod() const;
  Uri getUri() const;
  std::string getVersion() const;
  std::map<std::string, std::string> getHeaders() const;
  std::string getBody() const;
  std::pair<std::string, std::string> getHost();

private:
  enum parse_state {
    STATE_METHOD,
    STATE_URI,
    STATE_VERSION,
    STATE_NAME,
    STATE_COLON,
    STATE_VALUE,
    STATE_CRLF,
  };

  enum status status_;
  enum Http::method method_;
  Uri uri_;
  std::string version_;
  std::map<std::string, std::string> headers_;
  std::string body_;
};

#endif
