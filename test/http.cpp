#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "Http.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpResponseStatus.h"
#include "Uri.h"

int main(int argc, char **argv) {
  {
    std::cout << "## TEST REQ ##" << std::endl;
    if (argc == 2) {
      std::fstream req_file(argv[1]);
      if (req_file.fail()) {
        std::cerr << "fail to open req file" << std::endl;
        return 1;
      }
      std::string raw_req((std::istreambuf_iterator<char>(req_file)), (std::istreambuf_iterator<char>()));
      req_file.close();

      HttpRequest req(raw_req);
      std::cout << "## REQUEST ##" << std::endl;
      std::cout << "Method: ";
      switch (req.getMethod()) {
        case Http::UNKNOWN:
          std::cout << "NONE" << std::endl;
          break;
        case Http::GET:
          std::cout << "GET" << std::endl;
          break;
        case Http::POST:
          std::cout << "POST" << std::endl;
          break;
        case Http::DELETE:
          std::cout << "DELETE" << std::endl;
          break;
      }
      std::cout << "URI: " << req.getUri() << std::endl;
      std::cout << "Version: " << req.getVersion() << std::endl;
      std::cout << "Headers:" << std::endl;
      std::map<std::string, std::string> headers = req.getHeaders();
      for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it) {
        std::cout << "  [" << it->first << "]: " << it->second << std::endl;
      }
      std::cout << "Body:" << std::endl;
      std::cout << req.getBody() << std::endl;

      if (req.getStatus() == HttpRequest::S_OK) {
        std::cout << "Host: " << req.getHost().first << ":" << req.getHost().second << std::endl;
      }

      std::cout << "## RESPONSE ##" << std::endl;
      HttpResponse *resp;
      switch (req.getStatus()) {
        case HttpRequest::S_NONE: {
          resp = new HttpResponse(HttpResponseServerError::_500);
          break;
        }
        case HttpRequest::S_OK: {
          resp = new HttpResponse(HttpResponseSuccess::_200);
          break;
        }
        case HttpRequest::S_CONTINUE: {
          resp = new HttpResponse(HttpResponseInfo::_100);
          break;
        }
        case HttpRequest::S_BAD_REQUEST: {
          resp = new HttpResponse(HttpResponseClientError::_400);
          break;
        }
        case HttpRequest::S_NOT_IMPLEMENTED: {
          resp = new HttpResponse(HttpResponseServerError::_501);
          break;
        }
        case HttpRequest::S_HTTP_VERSION_NOT_SUPPORTED: {
          resp = new HttpResponse(HttpResponseServerError::_505);
          break;
        }
      }
      std::cout << resp->getRaw() << std::endl;
      delete resp;
    }
  }
  {
    std::cout << "## TEST URI ##" << std::endl;
    Uri empty("");
    std::cout << empty.getRaw() << std::endl;

    Uri good("http://google.com");
    std::cout << good.getRaw() << std::endl;

    Uri user("http://user:pass@test.com");
    std::cout << user.getRaw() << std::endl;

    Uri ip("http://123.123.123.123");
    std::cout << ip.getRaw() << std::endl;

    Uri port("http://localhost:878");
    std::cout << port.getRaw() << std::endl;

    Uri path("http://localhost.com/test/hello/world");
    std::cout << path.getRaw() << std::endl;

    Uri query("http://localhost.com?tse=234&asdf=egadfa");
    std::cout << query.getRaw() << std::endl;

    Uri fragment("http://localhost.com#frag");
    std::cout << fragment.getRaw() << std::endl;

    Uri relative("/test/hello?query=1213");
    std::cout << relative.getRaw() << std::endl;
  }
}