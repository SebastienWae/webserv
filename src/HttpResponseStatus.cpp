#include "HttpResponseStatus.h"

#include <vector>

std::vector<std::pair<std::string const, std::string const> > initResponseInfo() {
  std::vector<std::pair<std::string const, std::string const> > v;
  v.push_back(std::pair<std::string const, std::string>("100", "Continue"));
  v.push_back(std::pair<std::string const, std::string>("101", "Switching Protocols"));
  return v;
}
std::vector<std::pair<std::string const, std::string const> > HttpResponseInfo::status = initResponseInfo();

std::vector<std::pair<std::string const, std::string const> > initResponseSuccess() {
  std::vector<std::pair<std::string const, std::string const> > v;
  v.push_back(std::pair<std::string const, std::string>("200", "Ok"));
  v.push_back(std::pair<std::string const, std::string>("201", "Created"));
  v.push_back(std::pair<std::string const, std::string>("202", "Accepted"));
  v.push_back(std::pair<std::string const, std::string>("203", "Non-Authoritative Information"));
  v.push_back(std::pair<std::string const, std::string>("204", "No Content"));
  v.push_back(std::pair<std::string const, std::string>("205", "Reset Content"));
  v.push_back(std::pair<std::string const, std::string>("206", "Partial Content"));
  return v;
}
std::vector<std::pair<std::string const, std::string const> > HttpResponseSuccess::status = initResponseSuccess();

std::vector<std::pair<std::string const, std::string const> > initResponseRedir() {
  std::vector<std::pair<std::string const, std::string const> > v;
  v.push_back(std::pair<std::string const, std::string>("300", "Multiple Choices"));
  v.push_back(std::pair<std::string const, std::string>("301", "Moved Permanently"));
  v.push_back(std::pair<std::string const, std::string>("302", "Found"));
  v.push_back(std::pair<std::string const, std::string>("303", "See Other"));
  v.push_back(std::pair<std::string const, std::string>("304", "Not Modified"));
  v.push_back(std::pair<std::string const, std::string>("305", "Use Proxy"));
  v.push_back(std::pair<std::string const, std::string>("306", ""));
  v.push_back(std::pair<std::string const, std::string>("307", "Temporary Redirect"));
  return v;
}
std::vector<std::pair<std::string const, std::string const> > HttpResponseRedir::status = initResponseRedir();

std::vector<std::pair<std::string const, std::string const> > initResponseClientError() {
  std::vector<std::pair<std::string const, std::string const> > v;
  v.push_back(std::pair<std::string const, std::string>("400", "Bad Request"));
  v.push_back(std::pair<std::string const, std::string>("401", "Unauthorized"));
  v.push_back(std::pair<std::string const, std::string>("402", "Payment Required"));
  v.push_back(std::pair<std::string const, std::string>("403", "Forbiden"));
  v.push_back(std::pair<std::string const, std::string>("404", "Not Found"));
  v.push_back(std::pair<std::string const, std::string>("405", "Method Not Allowed"));
  v.push_back(std::pair<std::string const, std::string>("406", "Not Acceptable"));
  v.push_back(std::pair<std::string const, std::string>("407", "Proxy Authentication Required"));
  v.push_back(std::pair<std::string const, std::string>("408", "Request Timeout"));
  v.push_back(std::pair<std::string const, std::string>("409", "Conflict"));
  v.push_back(std::pair<std::string const, std::string>("410", "Gone"));
  v.push_back(std::pair<std::string const, std::string>("411", "Lenght Required"));
  v.push_back(std::pair<std::string const, std::string>("412", "Precondition Failed"));
  v.push_back(std::pair<std::string const, std::string>("413", "Request Entity Too Large"));
  v.push_back(std::pair<std::string const, std::string>("414", "Request-URI Too Long"));
  v.push_back(std::pair<std::string const, std::string>("415", "Unsuported Media Type"));
  v.push_back(std::pair<std::string const, std::string>("416", "Requested Range Not Satisfiable"));
  v.push_back(std::pair<std::string const, std::string>("417", "Expectation Failed"));
  return v;
}
std::vector<std::pair<std::string const, std::string const> > HttpResponseClientError::status
    = initResponseClientError();

std::vector<std::pair<std::string const, std::string const> > initResponseServerError() {
  std::vector<std::pair<std::string const, std::string const> > v;
  v.push_back(std::pair<std::string const, std::string>("500", "Internal Server Error"));
  v.push_back(std::pair<std::string const, std::string>("501", "Not Implemented"));
  v.push_back(std::pair<std::string const, std::string>("502", "Bad Gateway"));
  v.push_back(std::pair<std::string const, std::string>("503", "Service Unavailable"));
  v.push_back(std::pair<std::string const, std::string>("504", "Gateway Timeout"));
  v.push_back(std::pair<std::string const, std::string>("505", "HTTP Version Not Supported"));
  return v;
}
std::vector<std::pair<std::string const, std::string const> > HttpResponseServerError::status
    = initResponseServerError();