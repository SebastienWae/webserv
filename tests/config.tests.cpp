#include "../src/Config.h"

#include <__nullptr>

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
    HttpRequest req("GET / HTTP/1.1\r\nHost: 254.0.0.155:42422\r\n\r\n");
    ServerConfig const* sc = config.matchServerConfig(&req);
    CHECK(sc != nullptr);
    CHECK(sc->getHostname() == "254.0.0.155");
    CHECK(sc->getPort() == "42422");
    CHECK(sc->getHost() == "http://254.0.0.155:42422");
    File* error_500 = sc->getErrorPage(HttpResponseServerError::_500);
    CHECK(error_500 != nullptr);
    CHECK(error_500->exist());
    CHECK(error_500->isReadable());
    CHECK(error_500->getType() == File::REG);
    CHECK(error_500->getPath() == "/goinfre/swaegene/webserv/tests/configs/valid.conf");
    CHECK(sc->getErrorPage(HttpResponseServerError::_501) == nullptr);
    File* error_400 = sc->getErrorPage(HttpResponseClientError::_400);
    CHECK(error_400 != nullptr);
    CHECK(error_400->exist());
    CHECK(error_400->isReadable());
    CHECK(error_400->getType() == File::REG);
    CHECK(error_400->getPath() == "/goinfre/swaegene/webserv/tests/configs/valid.conf");
    CHECK(sc->getErrorPage(HttpResponseClientError::_401) == nullptr);
    CHECK(sc->getMaxBodySize() == 5000);
    // http://254.0.0.155:42422/
    {
      Uri uri("/");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/tests/configs/http");
      File* directory_page = r->getDirecoryPage();
      CHECK(directory_page != nullptr);
      CHECK(directory_page->exist());
      CHECK(directory_page->isReadable());
      CHECK(directory_page->getType() == File::REG);
      CHECK(directory_page->getPath() == "/goinfre/swaegene/webserv/tests/configs/http/test.html");
      CHECK(r->getUploadStore() == nullptr);
      CHECK_FALSE(r->isRedirection());
      CHECK(r->isAllowedMethod(Http::GET));
      CHECK_FALSE(r->isAllowedMethod(Http::POST));
      CHECK_FALSE(r->isAllowedMethod(Http::HEAD));
      CHECK_FALSE(r->isAllowedMethod(Http::DELETE));
      File* upload_store = r->getUploadStore();
      CHECK(upload_store == nullptr);
      File* file = r->matchFile(&uri);
      CHECK(file != nullptr);
      CHECK(file->exist());
      CHECK(file->isReadable());
      CHECK(file->getType() == File::DI);
      CHECK(file->getPath() == "/goinfre/swaegene/webserv/tests/configs/http");
      delete file;
    }
    // http://254.0.0.155:42422/test.html
    {
      Uri uri("/test.html");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/tests/configs/http");
      File* file = r->matchFile(&uri);
      CHECK(file != nullptr);
      CHECK(file->exist());
      CHECK(file->isReadable());
      CHECK(file->getType() == File::REG);
      CHECK(file->getPath() == "/goinfre/swaegene/webserv/tests/configs/http/test.html");
      delete file;
    }
    // http://254.0.0.155:42422/missing
    {
      Uri uri("/missing");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/tests/configs/http");
      File* file = r->matchFile(&uri);
      file = r->matchFile(&uri);
      CHECK(file == nullptr);
    }
    // http://254.0.0.155:42422/test.py
    {
      Uri uri("/test.py");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/tests/configs/http");
      File* cgi_dir = r->matchCGI(&uri);
      CHECK(cgi_dir != nullptr);
      CHECK(cgi_dir->exist());
      CHECK(cgi_dir->isReadable());
      CHECK(cgi_dir->isExecutable());
      CHECK(cgi_dir->getType() == File::DI);
      CHECK(cgi_dir->getPath() == "/goinfre/swaegene/webserv/tests/configs/http/cgi");
    }

    // cgi = r->matchCGI("/test.py");
    // CHECK(cgi != nullptr);
    // CHECK(cgi->exist());
    // CHECK(cgi->isExecutable());
    // CHECK(cgi->getType() == File::REG);
    // CHECK(cgi->getPath() == "/goinfre/swaegene/webserv/tests/configs/http/test.py");
    // cgi = r->matchCGI("/missing.py");
    // CHECK(cgi == nullptr);
    // http://254.0.0.155:42422/test_redir
    {
      Uri uri("/test_redir");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/test_redir");
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
      Uri uri("/subdir");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/subdir");
      CHECK(r->isAllowedMethod(Http::GET));
      CHECK_FALSE(r->isAllowedMethod(Http::POST));
      CHECK_FALSE(r->isAllowedMethod(Http::HEAD));
      CHECK(r->isAllowedMethod(Http::DELETE));
      CHECK_FALSE(r->isRedirection());
      File* file = r->matchFile(&uri);
      CHECK(file != nullptr);
      CHECK(file->exist());
      CHECK(file->isReadable());
      CHECK(file->getType() == File::DI);
      CHECK(file->getPath() == "/goinfre/swaegene/webserv/tests/configs/http/subdir");
      CHECK(r->isDirectoryListing());
      delete file;
    }
    // http://254.0.0.155:42422/subdir/hello/test.html
    {
      Uri uri("/subdir/hello/test.hml");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/subdir");
      File* root = r->getRoot();
      CHECK(root != nullptr);
      CHECK(root->exist());
      CHECK(root->isReadable());
      CHECK(root->getType() == File::DI);
      CHECK(root->getPath() == "/goinfre/swaegene/webserv/tests/configs/http/subdir");
      File* file = r->matchFile(&uri);
      CHECK(file == nullptr);
      delete file;
    }
    // http://254.0.0.155:42422/multi/
    {
      Uri uri("/multi/");
      Route* r = sc->matchRoute(&uri);
      CHECK(r != nullptr);
      CHECK(r->getLocation() == "/multi");
      CHECK(r->isAllowedMethod(Http::GET));
      CHECK(r->isAllowedMethod(Http::POST));
      CHECK_FALSE(r->isAllowedMethod(Http::HEAD));
      CHECK(r->isAllowedMethod(Http::DELETE));
      CHECK_FALSE(r->isRedirection());
      File* file = r->matchFile(&uri);
      CHECK(file != nullptr);
      CHECK(file->exist());
      CHECK(file->isReadable());
      CHECK(file->getType() == File::DI);
      CHECK(file->getPath() == "/goinfre/swaegene/webserv/tests/configs/http/sub/dir/test/");
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

  // test [:6868]

  // test []

  // test [test.com:40]
}

/*
- invalid host
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
