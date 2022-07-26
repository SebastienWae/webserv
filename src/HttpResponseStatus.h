#ifndef HTTPCODE_H
#define HTTPCODE_H

#include <string>
#include <vector>

struct HttpResponseInfo {
  enum code { _100, _101 };
  static std::vector<std::pair<std::string const, std::string const> > status;
};

struct HttpResponseSuccess {
  enum code { _200, _201, _202, _203, _204, _205, _206 };
  static std::vector<std::pair<std::string const, std::string const> > status;
};

struct HttpResponseRedir {
  enum code { _300, _301, _302, _303, _304, _305, _306, _307 };
  static std::vector<std::pair<std::string const, std::string const> > status;
};

struct HttpResponseClientError {
  enum code {
    _400,
    _401,
    _402,
    _403,
    _404,
    _405,
    _406,
    _407,
    _408,
    _409,
    _410,
    _411,
    _412,
    _413,
    _414,
    _415,
    _416,
    _417
  };
  static std::vector<std::pair<std::string const, std::string const> > status;
};

struct HttpResponseServerError {
  enum code { _500, _501, _502, _503, _504, _505 };
  static std::vector<std::pair<std::string const, std::string const> > status;
};

#endif
