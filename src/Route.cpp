#include "Route.h"

#include <string>

#include "Http.h"
#include "HttpResponseStatus.h"

Route::Route(std::string const& location) : location_(location), directory_listing_(false) {
  redirection_.second = NULL;
}

Route::~Route() {
  if (redirection_.second != NULL) {
    delete redirection_.second;
  }
}

Route::ParsingException::ParsingException(std::string const& msg) throw() : msg_(msg) {}
Route::ParsingException::~ParsingException() throw() {}
char const* Route::ParsingException::what() const throw() { return msg_.c_str(); }

void Route::parse(std::string const& line) {  // NOLINT
  std::string::size_type sep = line.find('=');
  if (sep != std::string::npos) {
    std::string key = line.substr(0, sep);
    std::string value = line.substr(sep + 1);

    if (key == "root") {
      // TODO: validate path
      root_ = value;
    } else if (key == "directory_page") {
      // TODO: validate path
      directory_page_ = value;
    } else if (key == "directory_listing") {
      // TODO: validate path
      if (value == "on") {
        directory_listing_ = true;
      } else if (value != "off") {
        throw ParsingException("Config file error at line: " + line);
      }
    } else if (key == "upload_store") {
      // TODO: validate path
      upload_store_ = value;
    } else if (key == "allow") {
      while (std::string::size_type pos = value.find(',')) {
        if (pos == std::string::npos) {
          break;
        }
        Http::method method = Http::UNKNOWN;
        if (value.substr(0, pos) == "GET") {
          method = Http::GET;
        } else if (value.substr(0, pos) == "HEAD") {
          method = Http::HEAD;
        } else if (value.substr(0, pos) == "POST") {
          method = Http::POST;
        } else if (value.substr(0, pos) == "DELETE") {
          method = Http::DELETE;
        }
        if (method != Http::UNKNOWN) {
          allowed_methods_.insert(method);
        } else {
          throw ParsingException("Config file error at line: " + line);
        }
        value = value.substr(pos + 1);
      }
    } else if (key == "redirection") {
      if (redirection_.second == NULL) {
        sep = value.find(' ');
        if (sep == std::string::npos) {
          throw ParsingException("Config file error at line: " + line);
        }
        std::string code = value.substr(0, sep);
        std::string path = value.substr(sep + 1);

        int code_i = std::atoi(code.c_str());
        if ((code_i - 300) >= 0 && (code_i - 300) < (307 - 300)) {
          redirection_.first = static_cast<HttpResponseRedir::code>(code_i - 300);
        } else {
          throw ParsingException("Config file error at line: " + line);
        }
        try {
          Uri* uri = new Uri(path);
          if (uri->getType() == Uri::TYPE_ABSOLUTE || uri->getType() == Uri::TYPE_RELATIVE || uri->getQuery().empty()) {
            redirection_.second = uri;
          } else {
            throw ParsingException("Config file error at line: " + line);
          }
        } catch (Uri::UriParsingException) {
          throw ParsingException("Config file error at line: " + line);
        }
      } else {
        throw ParsingException("Config file error at line: " + line);
      }
    } else if (key == "cgi") {
      // TODO: validate path & ext
      sep = line.find(' ');
      std::string ext = value.substr(0, sep);
      std::string path = value.substr(sep + 1);
      if (cgi_.find(ext) == cgi_.end()) {
        std::pair<std::string, std::string> new_cgi(ext, path);
        cgi_.insert(new_cgi);
      } else {
        throw ParsingException("Config file error at line: " + line);
      }
    } else {
      throw ParsingException("Config file error at line: " + line);
    }
  } else {
    throw ParsingException("Config file error at line: " + line);
  }
}

void Route::verify() const {
  if (root_.empty()) {
    throw ParsingException("All routes must have a root directory");
  }
}

std::string const& Route::getLocation() const { return location_; }

std::string const& Route::getRoot() const { return root_; }

std::string const& Route::getDirectoryPage() const { return directory_page_; }

std::string const& Route::getUploadStore() const { return upload_store_; }

bool Route::isRedirection() const { return redirection_.second != NULL; }

std::pair<HttpResponseRedir::code, Uri*> const& Route::getRedirection() const { return redirection_; }

bool Route::isAllowedMethod(enum Http::method method) const {
  std::set<enum Http::method>::iterator m = allowed_methods_.find(method);
  return m != allowed_methods_.end();
}

// TODO
std::string Route::matchCGI(std::string const& file) const {
  (void)file;
  return "";
}