#ifndef HTTPREQUEST_H
#define HTTPREQUEST_H

#include <_types/_uint8_t.h>

#include <cstddef>
#include <ctime>
#include <exception>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "Config.h"
#include "Http.h"
#include "Uri.h"

#define GIGA 1000000000
#define HEX_BASE 16

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
    S_LENGTH_REQUIRED,
    S_REQUEST_ENTITY_TOO_LARGE
  };

  typedef std::map<std::string, enum Http::method> MethodMap;
  static const MethodMap method_map;

  HttpRequest(std::vector<uint8_t> const& data, Config const& config);

  void addChunk(std::vector<uint8_t> const& chunk, std::size_t max_body_size);

  ~HttpRequest();

  enum status getStatus() const;
  std::time_t const& getTime() const;
  enum Http::method getMethod() const;
  Uri const* getUri() const;
  std::map<std::string, std::string> const& getHeaders() const;
  std::vector<uint8_t> const& getBody() const;
  std::string getHost() const;

  bool isFileUpload() const;

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

  enum status status_;
  std::time_t time_;
  enum Http::method method_;
  Uri* uri_;
  std::map<std::string, std::string> headers_;
  std::vector<uint8_t> body_;
  std::vector<uint8_t> chunks_buff_;
  std::size_t content_length_;
  bool is_chunked_;

  void parseChunks(std::size_t max_body_size);
};

#endif
