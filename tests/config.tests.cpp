#include "../src/Config.h"

#include <sys/types.h>

#include <__nullptr>
#include <cstddef>
#include <limits>
#include <vector>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("valid config") {
  Config config("configs/valid.conf");

  std::set<std::string> ports = config.getPorts();
  CHECK(ports.size() == 4);
  CHECK(ports.find("42422") != ports.end());
  CHECK(ports.find("80") != ports.end());
  CHECK(ports.find("6868") != ports.end());
  CHECK(ports.find("40") != ports.end());

  // test [254.0.0.155:42422]
  {
    CAPTURE("[254.0.0.155:42422]");
    std::string s = "GET / HTTP/1.1\r\nHost: 254.0.0.155:42422\r\n\r\n";
    std::vector<uint8_t> r(s.begin(), s.end());
    HttpRequest req(r);
    ServerConfig const* sc = config.matchServerConfig(&req);
    CHECK(sc != nullptr);
    CHECK(sc->getHostname() == "254.0.0.155");
    CHECK(sc->getPort() == "42422");
    CHECK(sc->getHost() == "254.0.0.155:42422");
    File* error_500 = sc->getErrorPage(HttpResponseServerError::_500);
    CHECK(error_500 != nullptr);
    CHECK(error_500->exist());
    CHECK(error_500->isReadable());
    CHECK(error_500->getType() == File::REG);
    CHECK(error_500->getPath() == "/goinfre/swaegene/webserv/http/server_error.html");
    CHECK(sc->getErrorPage(HttpResponseServerError::_501) == nullptr);
    File* error_400 = sc->getErrorPage(HttpResponseClientError::_400);
    CHECK(error_400 != nullptr);
    CHECK(error_400->exist());
    CHECK(error_400->isReadable());
    CHECK(error_400->getType() == File::REG);
    CHECK(error_400->getPath() == "/goinfre/swaegene/webserv/http/client_error.html");
    CHECK(sc->getErrorPage(HttpResponseClientError::_401) == nullptr);
    CHECK(sc->getMaxBodySize() == 5000);
    // http://254.0.0.155:42422/
    {
      CAPTURE("http://254.0.0.155:42422/");
      Uri uri("/");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/http");
      File* directory_page = r->getDirecoryPage();
      CHECK(directory_page != nullptr);
      CHECK(directory_page->exist());
      CHECK(directory_page->isReadable());
      CHECK(directory_page->getType() == File::REG);
      CHECK(directory_page->getPath() == "/goinfre/swaegene/webserv/http/dir.html");
      CHECK(r->getUploadStore() == nullptr);
      CHECK_FALSE(r->isRedirection());
      CHECK(r->isAllowedMethod(Http::GET));
      CHECK_FALSE(r->isAllowedMethod(Http::POST));
      CHECK_FALSE(r->isAllowedMethod(Http::HEAD));
      CHECK_FALSE(r->isAllowedMethod(Http::DELETE));
      File* upload_store = r->getUploadStore();
      CHECK(upload_store == nullptr);
      File* file = nullptr;
      CHECK_NOTHROW(file = r->matchFile(&uri));
      CHECK(file != nullptr);
      CHECK(file->exist());
      CHECK(file->isReadable());
      CHECK(file->getType() == File::DI);
      CHECK(file->getPath() == "/goinfre/swaegene/webserv/http");
      delete file;
    }
    // http://254.0.0.155:42422/test.html
    {
      CAPTURE("http://254.0.0.155:42422/test.html");
      Uri uri("/test.html");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/http");
      File* file = nullptr;
      CHECK_NOTHROW(file = r->matchFile(&uri));
      CHECK(file != nullptr);
      CHECK(file->exist());
      CHECK(file->isReadable());
      CHECK(file->getType() == File::REG);
      CHECK(file->getPath() == "/goinfre/swaegene/webserv/http/test.html");
      delete file;
    }
    // http://254.0.0.155:42422/missing.html
    {
      CAPTURE("http://254.0.0.155:42422/missing.html");
      Uri uri("/missing.html");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/http");
      CHECK_THROWS_AS(r->matchFile(&uri), Route::NotFoundException);
    }
    // http://254.0.0.155:42422/cgi/test.py
    {
      CAPTURE("http://254.0.0.155:42422/cgi/test.py");
      Uri uri("/cgi/test.py");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/http");
      File* cgi = nullptr;
      CHECK_NOTHROW(cgi = r->matchFile(&uri));
      CHECK(cgi != nullptr);
      CHECK(cgi->exist());
      CHECK(cgi->isExecutable());
      CHECK(cgi->getType() == File::REG);
      CHECK(cgi->getPath() == "/goinfre/swaegene/webserv/http/cgi/test.py");
    }
    // http://254.0.0.155:42422/cgi/missing.py
    {
      CAPTURE("http://254.0.0.155:42422/cgi/missing.py");
      Uri uri("/cgi/missing.py");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/http");
      CHECK_THROWS_AS(r->matchCGI(&uri), Route::NotFoundException);
    }
    // http://254.0.0.155:42422/cgi/perm.py
    {
      CAPTURE("http://254.0.0.155:42422/cgi/perm.py");
      Uri uri("/cgi/perm.py");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/http");
      CHECK_THROWS_AS(r->matchCGI(&uri), Route::ForbiddenException);
    }
    // http://254.0.0.155:42422/relative_redir
    {
      CAPTURE("http://254.0.0.155:42422/relative_redir");
      Uri uri("/relative_redir");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/relative_redir");
      CHECK_FALSE(r->isAllowedMethod(Http::GET));
      CHECK_FALSE(r->isAllowedMethod(Http::POST));
      CHECK_FALSE(r->isAllowedMethod(Http::HEAD));
      CHECK_FALSE(r->isAllowedMethod(Http::DELETE));
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/");
      CHECK(r->isRedirection());
      CHECK(r->getRedirection().first == HttpResponseRedir::_302);
      Uri* redir = r->getRedirection().second;
      CHECK(redir != nullptr);
      CHECK(redir->getType() == Uri::TYPE_RELATIVE);
      CHECK(redir->getPath() == "/other_page.html");
    }
    // http://254.0.0.155:42422/absolute_redir
    {
      CAPTURE("http://254.0.0.155:42422/absolute_redir");
      Uri uri("/absolute_redir");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/absolute_redir");
      CHECK_FALSE(r->isAllowedMethod(Http::GET));
      CHECK_FALSE(r->isAllowedMethod(Http::POST));
      CHECK_FALSE(r->isAllowedMethod(Http::HEAD));
      CHECK_FALSE(r->isAllowedMethod(Http::DELETE));
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/");
      CHECK(r->isRedirection());
      CHECK(r->getRedirection().first == HttpResponseRedir::_301);
      Uri* redir = r->getRedirection().second;
      CHECK(redir != nullptr);
      CHECK(redir->getType() == Uri::TYPE_ABSOLUTE);
      CHECK(redir->getHost() == "google.com");
      CHECK(redir->getPath() == "/test");
    }
    // http://254.0.0.155:42422/subdir
    {
      CAPTURE("http://254.0.0.155:42422/subdir");
      Uri uri("/subdir");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/subdir");
      CHECK(r->isAllowedMethod(Http::GET));
      CHECK_FALSE(r->isAllowedMethod(Http::POST));
      CHECK_FALSE(r->isAllowedMethod(Http::HEAD));
      CHECK(r->isAllowedMethod(Http::DELETE));
      CHECK_FALSE(r->isRedirection());
      File* file = nullptr;
      CHECK_NOTHROW(file = r->matchFile(&uri));
      CHECK(file != nullptr);
      CHECK(file->exist());
      CHECK(file->isReadable());
      CHECK(file->getType() == File::DI);
      CHECK(file->getPath() == "/goinfre/swaegene/webserv/http/subdir");
      CHECK(r->isDirectoryListing());
      delete file;
    }
    // http://254.0.0.155:42422/subdir/hello/test.html
    {
      CAPTURE("http://254.0.0.155:42422/subdir/hello/test.html");
      Uri uri("/subdir/hello/test.hml");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/subdir");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/http/subdir");
      CHECK_THROWS_AS(r->matchFile(&uri), Route::NotFoundException);
    }
    // http://254.0.0.155:42422/multi/
    {
      CAPTURE("http://254.0.0.155:42422/multi");
      Uri uri("/multi/");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/multi");
      CHECK(r->isAllowedMethod(Http::GET));
      CHECK(r->isAllowedMethod(Http::POST));
      CHECK_FALSE(r->isAllowedMethod(Http::HEAD));
      CHECK(r->isAllowedMethod(Http::DELETE));
      CHECK_FALSE(r->isRedirection());
      File* file = nullptr;
      CHECK_NOTHROW(file = r->matchFile(&uri));
      CHECK(file != nullptr);
      CHECK(file->exist());
      CHECK(file->isReadable());
      CHECK(file->getType() == File::DI);
      CHECK(file->getPath() == "/goinfre/swaegene/webserv/http/subdir/nested");
      CHECK_FALSE(r->isDirectoryListing());
      delete file;
      File* upload_store = r->getUploadStore();
      CHECK(upload_store != nullptr);
      CHECK(upload_store->exist());
      CHECK(upload_store->isWritable());
      CHECK(upload_store->getType() == File::DI);
    }
  }

  // test [test.com]
  {
    CAPTURE("[test.com]");
    std::string s = "HEAD / HTTP/1.1\r\nHost: test.com\r\n\r\n";
    std::vector<uint8_t> r(s.begin(), s.end());
    HttpRequest req(r);
    ServerConfig const* sc = config.matchServerConfig(&req);
    CHECK(sc != nullptr);
    CHECK(sc->getHostname() == "test.com");
    CHECK(sc->getPort() == "80");
    CHECK(sc->getHost() == "test.com:80");
    CHECK(sc->getErrorPage(HttpResponseClientError::_400) == nullptr);
    CHECK(sc->getErrorPage(HttpResponseServerError::_500) == nullptr);
    CHECK(sc->getMaxBodySize() == std::numeric_limits<std::size_t>::max());
  }

  // test [:6868]
  {
    CAPTURE("[:6868]");
    std::string s = "POST / HTTP/1.1\r\nHost: localhost:6868\r\n\r\n";
    std::vector<uint8_t> r(s.begin(), s.end());
    HttpRequest req(r);
    ServerConfig const* sc = config.matchServerConfig(&req);
    CHECK(sc != nullptr);
    CHECK(sc->getHostname() == "localhost");
    CHECK(sc->getPort() == "6868");
    CHECK(sc->getHost() == "localhost:6868");
    CHECK(sc->getErrorPage(HttpResponseClientError::_401) == nullptr);
    CHECK(sc->getErrorPage(HttpResponseServerError::_501) == nullptr);
    CHECK(sc->getMaxBodySize() == 0);
  }

  // test []
  {
    CAPTURE("[]");
    std::string s = "GET / HTTP/1.1\r\nHost: localhost:80\r\n\r\n";
    std::vector<uint8_t> r(s.begin(), s.end());
    HttpRequest req(r);
    ServerConfig const* sc = config.matchServerConfig(&req);
    CHECK(sc != nullptr);
    CHECK(sc->getHostname() == "localhost");
    CHECK(sc->getPort() == "80");
    CHECK(sc->getHost() == "localhost:80");
    CHECK(sc->getErrorPage(HttpResponseClientError::_403) == nullptr);
    CHECK(sc->getErrorPage(HttpResponseServerError::_503) == nullptr);
    CHECK(sc->getMaxBodySize() == std::numeric_limits<std::size_t>::max());
  }

  // test [test.com:40]
  {
    CAPTURE("[test.com:40]");
    std::string s = "DELETE / HTTP/1.1\r\nHost: test.com:40\r\n\r\n";
    std::vector<uint8_t> r(s.begin(), s.end());
    HttpRequest req(r);
    ServerConfig const* sc = config.matchServerConfig(&req);
    CHECK(sc != nullptr);
    CHECK(sc->getHostname() == "test.com");
    CHECK(sc->getPort() == "40");
    CHECK(sc->getHost() == "test.com:40");
    CHECK(sc->getErrorPage(HttpResponseClientError::_404) == nullptr);
    CHECK(sc->getErrorPage(HttpResponseServerError::_504) == nullptr);
    CHECK(sc->getMaxBodySize() == std::numeric_limits<std::size_t>::max());
  }

  // test google.com
  {
    CAPTURE("http://google.com");
    std::string s = "DELETE / HTTP/1.1\r\nHost: google.com\r\n\r\n";
    std::vector<uint8_t> r(s.begin(), s.end());
    HttpRequest req(r);
    ServerConfig const* sc = config.matchServerConfig(&req);
    CHECK(sc != nullptr);
    CHECK(sc->getHostname() == "254.0.0.155");
    CHECK(sc->getPort() == "42422");
    CHECK(sc->getHost() == "254.0.0.155:42422");
  }
}

TEST_CASE("invalid host") {
  // missing
  CHECK_THROWS_AS(new Config("configs/missing_host.conf"), Config::ParsingException);

  // invalid format
  CHECK_THROWS_AS(new Config("configs/invalid_format_host.conf"), Config::ParsingException);

  // invalid char
  CHECK_THROWS_AS(new Config("configs/invalid_char_host.conf"), Config::ParsingException);
}

/*
- invalid max body size
- invalid error
  - code
  - path
- invalid rout
  - path
  - root
    - missing
    - wrong
  - allow
  - directory_page
  - cgi
    - ext
    - path
  - redirection
    - code
    - path
  - upload_store
*/
