#include "Route.h"

#include <__nullptr>
#include <cctype>
#include <cstddef>
#include <exception>
#include <new>
#include <string>

#include "Http.h"
#include "HttpResponseStatus.h"

Route::Route(std::string const& location)
    : location_(location), root_(NULL), directory_listing_(false), directory_page_(NULL), upload_store_(NULL) {
  redirection_.second = NULL;
}

Route::~Route() {
  if (redirection_.second != NULL) {
    delete redirection_.second;
  }
  if (root_ != NULL) {
    delete root_;
  }
  if (directory_page_ != NULL) {
    delete directory_page_;
  }
  if (upload_store_ != NULL) {
    delete upload_store_;
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
      File* root_dir = new File(value);
      if (root_dir->getType() == File::DIR && root_dir->isReadable()) {
        root_ = root_dir;
      } else {
        delete root_dir;
        throw ParsingException("Config file error at line: " + line);
      }
    } else if (key == "directory_page") {
      File* page = new File(value);
      if (page->getType() == File::REG && page->isReadable() && page->getIStream() != NULL) {
        directory_page_ = page;
      } else {
        delete page;
        throw ParsingException("Config file error at line: " + line);
      }
    } else if (key == "directory_listing") {
      if (value == "on") {
        directory_listing_ = true;
      } else if (value != "off") {
        throw ParsingException("Config file error at line: " + line);
      }
    } else if (key == "upload_store") {
      File* store = new File(value);
      if (store->getType() == File::DIR && store->isReadable() && store->isWritable()) {
        directory_page_ = store;
      } else {
        delete store;
        throw ParsingException("Config file error at line: " + line);
      }
    } else if (key == "allow") {
      while (std::string::size_type pos = value.find(',')) {
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
        if (pos == std::string::npos) {
          break;
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
      sep = value.find(' ');
      std::string ext = value.substr(0, sep);
      if (!ext.empty() && ext.at(0) == '.') {
        for (std::string::iterator it = ext.begin() + 1; it != ext.end(); ++it) {
          if (std::isalnum(*it) == 0) {
            throw ParsingException("Config file error at line: " + line);
          }
        }
        std::string path = value.substr(sep + 1);
        if (cgi_.find(ext) == cgi_.end()) {
          File* cgi_dir = new File(path);
          if (cgi_dir->getType() == File::DIR && cgi_dir->isReadable() && cgi_dir->isExecutable()) {
            std::pair<std::string, File*> new_cgi(ext, cgi_dir);
            cgi_.insert(new_cgi);
          } else {
            delete cgi_dir;
            throw ParsingException("Config file error at line: " + line);
          }
        } else {
          throw ParsingException("Config file error at line: " + line);
        }
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
  if (root_ == NULL) {
    throw ParsingException("All routes must have a root directory");
  }
}

std::string const& Route::getLocation() const { return location_; }

File* Route::getRoot() const { return root_; }

File* Route::getDirecoryPage() const { return directory_page_; }

File* Route::getUploadStore() const { return upload_store_; }

bool Route::isRedirection() const { return redirection_.second != NULL; }

std::pair<HttpResponseRedir::code, Uri*> const& Route::getRedirection() const { return redirection_; }

bool Route::isAllowedMethod(enum Http::method method) const {
  std::set<enum Http::method>::iterator m = allowed_methods_.find(method);
  return m != allowed_methods_.end();
}

bool Route::isDirectoryListing() const { return directory_listing_; }

File* Route::matchFile(Uri const* uri) const {
  std::string file_path;
  std::string uri_path = uri->getDecodedPath();
  std::string route_path = this->location_;
  if (uri_path.size() > route_path.size()) {
    file_path = uri_path.substr(route_path.size());
  }
  std::string absolute_path = root_->getPath() + "/" + file_path;
  File* file = new File(absolute_path);
  if (file->exist()) {
    return file;
  }
  delete file;
  return nullptr;
}

File* Route::matchCGI(std::string const& file) const {
  std::string::size_type sep = file.find_last_of('.');
  if (sep != std::string::npos) {
    std::string ext = file.substr(sep);
    std::map<std::string, File*>::const_iterator cgi_dir = cgi_.find(ext);

    if (cgi_dir != cgi_.end()) {
      return (cgi_dir->second);
    }
  }
  return nullptr;
}