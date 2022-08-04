#include "../src/HttpRequest.h"

#include <fstream>
#include <list>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("simple get") {
  std::stringstream buffer;
  std::ifstream file("requests/simple_get");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_OK);
  CHECK(req.getMethod() == Http::GET);
  CHECK(req.getBody() == "");
  CHECK(req.getHost() == "github.com:80");

  std::map<std::string, std::string> const& headers = req.getHeaders();
  CHECK(headers.size() == 1);
}

TEST_CASE("complete get") {
  std::stringstream buffer;
  std::ifstream file("requests/complete_get");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_OK);
  CHECK(req.getMethod() == Http::GET);
  CHECK(req.getBody() == "");
  CHECK(req.getHost() == "github.com:80");

  std::map<std::string, std::string> const& headers = req.getHeaders();
  CHECK(headers.size() == 16);
}

TEST_CASE("missing method") {
  std::stringstream buffer;
  std::ifstream file("requests/missing_method");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_BAD_REQUEST);
}

TEST_CASE("wrong method") {
  std::stringstream buffer;
  std::ifstream file("requests/wrong_method");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_NOT_IMPLEMENTED);
}

TEST_CASE("missing uri") {
  std::stringstream buffer;
  std::ifstream file("requests/missing_uri");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_BAD_REQUEST);
}

TEST_CASE("invalid uri") {
  std::stringstream buffer;
  std::ifstream file("requests/invalid_uri");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_BAD_REQUEST);
}

TEST_CASE("missing version") {
  std::stringstream buffer;
  std::ifstream file("requests/missing_version");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_BAD_REQUEST);
}

TEST_CASE("invalid version") {
  std::stringstream buffer;
  std::ifstream file("requests/invalid_version");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_HTTP_VERSION_NOT_SUPPORTED);
}

TEST_CASE("missing host") {
  std::stringstream buffer;
  std::ifstream file("requests/missing_host");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_BAD_REQUEST);
}

TEST_CASE("invalid host") {
  std::stringstream buffer;
  std::ifstream file("requests/invalid_host");
  REQUIRE(file.is_open());
  buffer << file.rdbuf();
  HttpRequest req(buffer.str());
  CHECK(req.getStatus() == HttpRequest::S_BAD_REQUEST);
}