#ifndef URI_H
#define URI_H

#include <iterator>
#include <map>
#include <string>
#include <vector>

class Uri {
public:
  enum type { TYPE_NONE, TYPE_ABSOLUTE, TYPE_RELATIVE };

  class UriParsingException : public std::exception {};

  Uri();
  Uri(std::string const& uri) throw(UriParsingException);

  enum type getType() const;

  std::string getUserInfo() const;
  std::string getHost() const;
  std::string getPort() const;
  std::string getQuery() const;
  std::string getRaw() const;
  std::string getPath() const;

private:
  enum parse_state {
    STATE_IDENTIFY,
    STATE_SCHEME,
    STATE_HOST,
    STATE_HOST_TYPE,
    STATE_USERINFO,
    STATE_HOSTNAME,
    STATE_IPV4ADDRESS,
    STATE_PORT,
    STATE_PATH,
    STATE_QUERY,
    STATE_FRAGMENT
  };

  enum type type_;

  std::string scheme_;
  std::string userinfo_;
  std::string host_;
  std::string port_;
  std::string path_;
  std::string query_;
  std::string fragment_;
};

#endif
