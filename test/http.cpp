#include <i386/types.h>

#include <exception>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

#include "../src/Http.h"
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

    std::string uri;
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http://google.com";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http://user:pass@test.com";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http://123.123.123.123";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http://localhost:878";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http://localhost.com/test/hello/world";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http://localhost.com?ts";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http://localhost.com#frag";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "//test";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "/test/hello?query=1213";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "/test//test//test////test";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "askdlfj/test/hello";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "?hello";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http:localhost";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "http://user@/test";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }

    uri = "ftp://test.com";
    try {
      Uri u(uri);
      std::cout << "success: " << u.getRaw() << std::endl;
    } catch (std::exception) {
      std::cout << "error: " << uri << std::endl;
    }
  }
}