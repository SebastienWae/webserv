#ifndef HTTP_H
#define HTTP_H

#define SP ' '
#define HT '\t'
#define CR '\r'
#define LF '\n'
#define CRLF "\r\n"
#define HTTP_VERSION "HTTP/1.1"
namespace Http {
  enum method { UNKNOWN, GET, POST, DELETE };
}

#endif