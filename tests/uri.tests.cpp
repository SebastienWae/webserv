#include "../src/Uri.h"

#include <list>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

struct t {
  t(Uri::type type, const std::string& raw, const std::string& user_info, const std::string& host,
    const std::string& port, const std::string& query, const std::string& path)
      : type_(type), raw_(raw), user_info_(user_info), host_(host), port_(port), query_(query), path_(path) {}
  Uri::type type_;
  std::string raw_;
  std::string user_info_;
  std::string host_;
  std::string port_;
  std::string query_;
  std::string path_;
};

void okCheck(const t& test) {
  Uri uri(test.raw_);
  CHECK(uri.getType() == test.type_);
  CHECK(uri.getUserInfo() == test.user_info_);
  CHECK(uri.getHost() == test.host_);
  CHECK(uri.getPort() == test.port_);
  CHECK(uri.getQuery() == test.query_);
  CHECK(uri.getPath() == test.path_);
}

TEST_CASE("valid absolute") {
  std::list<t> tests;
  // raw | user_info | host | port | query | path
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://google.com", "", "google.com", "", "", "/"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://google.com/", "", "google.com", "", "", "/"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://localhost", "", "localhost", "", "", "/"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://test.com:123", "", "test.com", "123", "", "/"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://123.123.123.123", "", "123.123.123.123", "", "", "/"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://1.2.3.4:111", "", "1.2.3.4", "111", "", "/"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://h.test/sub/dir", "", "h.test", "", "", "/sub/dir"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://von.biz/ipsam/quis-aperiam/hic-voluptatem.html", "", "von.biz", "", "",
                    "/ipsam/quis-aperiam/hic-voluptatem.html"));

  for (std::list<t>::iterator it = tests.begin(); it != tests.end(); ++it) {
    CAPTURE((*it).raw_);
    okCheck(*it);
  }
}

TEST_CASE("valid relative") {
  std::list<t> tests;
  // raw | user_info | host | port | query | path
  tests.push_back(t(Uri::TYPE_RELATIVE, "/", "", "", "", "", "/"));
  tests.push_back(t(Uri::TYPE_RELATIVE, "/test", "", "", "", "", "/test"));
  tests.push_back(t(Uri::TYPE_RELATIVE, "/d//", "", "", "", "", "/d//"));
  tests.push_back(
      t(Uri::TYPE_RELATIVE, "/tes////tt-..-///d/d/d//d//d//d/t", "", "", "", "", "/tes////tt-..-///d/d/d//d//d//d/t"));

  for (std::list<t>::iterator it = tests.begin(); it != tests.end(); ++it) {
    CAPTURE((*it).raw_);
    okCheck(*it);
  }
}

TEST_CASE("valid query") {
  std::list<t> tests;
  // raw | user_info | host | port | query | path
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://localhost?hello", "", "localhost", "", "hello", "/"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://localhost:1/?hello", "", "localhost", "1", "hello", "/"));
  tests.push_back(
      t(Uri::TYPE_ABSOLUTE, "http://localhost?hello&ttt=123;;i/d/d", "", "localhost", "", "hello&ttt=123;;i/d/d", "/"));
  tests.push_back(
      t(Uri::TYPE_ABSOLUTE, "http://localhost?hello&ttt=123;;i/d/d", "", "localhost", "", "hello&ttt=123;;i/d/d", "/"));
  tests.push_back(t(Uri::TYPE_RELATIVE, "/test?hello&dd&d.dv", "", "", "", "hello&dd&d.dv", "/test"));
  tests.push_back(t(Uri::TYPE_RELATIVE, "/test/dir/?hello&dd&d.dv", "", "", "", "hello&dd&d.dv", "/test/dir/"));
  tests.push_back(t(Uri::TYPE_ABSOLUTE, "http://a/b/c/d;p?q", "", "a", "", "q", "/b/c/d;p"));

  for (std::list<t>::iterator it = tests.begin(); it != tests.end(); ++it) {
    CAPTURE((*it).raw_);
    okCheck(*it);
  }
}

TEST_CASE("decode") {
  std::list<t> tests;
  // raw | user_info | host | port | query | path
  tests.push_back(
      t(Uri::TYPE_RELATIVE, "/as%20a%22%3F%3F%3F..%25%25%20%25%20dees", "", "", "", "", "/as a\"???..%% % dees"));
  tests.push_back(t(Uri::TYPE_RELATIVE, "/%3F%20and%20the%20Mysterians", "", "", "", "", "/? and the Mysterians"));
  tests.push_back(t(Uri::TYPE_RELATIVE, "/%3F+and+the+Mysterians", "", "", "", "", "/? and the Mysterians"));

  for (std::list<t>::iterator it = tests.begin(); it != tests.end(); ++it) {
    CAPTURE((*it).raw_);
    Uri uri((*it).raw_);
    CHECK(uri.getDecodedPath() == (*it).path_);
  }
}

TEST_CASE("invalid") {
  std::list<std::string> tests;
  // raw | user_info | host | port | query | path
  tests.push_back("");
  tests.push_back("http:test.com");
  tests.push_back("http:test.com");
  tests.push_back("http:/test.com");
  tests.push_back("http:://test.com");
  tests.push_back("http//test.com");
  tests.push_back("ftp://test.com");
  tests.push_back("https://test.com");
  tests.push_back("http://test.com:");
  tests.push_back("http://test.com:/");
  tests.push_back("http://test.com:/test");
  tests.push_back("http://test.com//");
  tests.push_back("asdfa");
  tests.push_back("/a /");
  tests.push_back("/a ");
  tests.push_back("/a ");
  tests.push_back("/?d d");

  for (std::list<std::string>::iterator it = tests.begin(); it != tests.end(); ++it) {
    CAPTURE(*it);
    CHECK_THROWS_AS(Uri v(*it), Uri::UriParsingException);
  }
}