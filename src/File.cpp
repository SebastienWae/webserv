#include "File.h"

#include <sys/_types/_timespec.h>
#include <sys/stat.h>

#include <fstream>
#include <sstream>

File::File(std::string const& path) : path_(path), input_stream_(NULL), output_stream_(NULL) {
  last_time.tv_sec = 0;
  last_time.tv_nsec = 0;
  stat_.st_ctimespec.tv_sec = 0;
  stat_.st_ctimespec.tv_nsec = 0;
  stat();
}

File::~File() {
  if (input_stream_ != NULL) {
    input_stream_->close();
    delete input_stream_;
  }
  if (output_stream_ != NULL) {
    output_stream_->close();
    delete output_stream_;
  }
}

bool File::stat() {
  if (stat_.st_ctimespec.tv_sec != 0 && stat_.st_ctimespec.tv_nsec != 0) {
    last_time.tv_sec = stat_.st_ctimespec.tv_nsec;
    last_time.tv_nsec = stat_.st_ctimespec.tv_nsec;
  }
  if (::stat(path_.c_str(), &stat_) < 0) {
    error_ = std::strerror(errno);
    return false;
  }
  last_time.tv_sec = 0;
  last_time.tv_nsec = 0;
  stat_.st_ctimespec.tv_sec = 0;
  stat_.st_ctimespec.tv_nsec = 0;
  return true;
}

bool File::exist() { return stat(); }

std::string const& File::getPath() const { return path_; }

bool File::isReadable() {
  if (stat()) {
    return (stat_.st_mode & S_IRUSR) != 0;
  }
  return false;
}

bool File::isWritable() {
  if (stat()) {
    return (stat_.st_mode & S_IWUSR) != 0;
  }
  return false;
}

bool File::isExecutable() {
  if (stat()) {
    return (stat_.st_mode & S_IXUSR) != 0;
  }
  return false;
}

enum File::type File::getType() {
  if (stat()) {
    if ((stat_.st_mode & S_IFIFO) != 0) {
      return FIFO;
    }
    if ((stat_.st_mode & S_IFCHR) != 0) {
      return CHR;
    }
    if ((stat_.st_mode & S_IFDIR) != 0) {
      return DIR;
    }
    if ((stat_.st_mode & S_IFBLK) != 0) {
      return BLK;
    }
    if ((stat_.st_mode & S_IFREG) != 0) {
      return REG;
    }
    if ((stat_.st_mode & S_IFLNK) != 0) {
      return LNK;
    }
    if ((stat_.st_mode & S_IFSOCK) != 0) {
      return SOCK;
    }
    if ((stat_.st_mode & S_IFWHT) != 0) {
      return WHT;
    }
  }
  return NONE;
}

struct timespec File::getLastAccess() {
  if (stat()) {
    return stat_.st_atimespec;
  }
  return (struct timespec){0, 0};
}

struct timespec File::getLastModification() {
  if (stat()) {
    return stat_.st_mtimespec;
  }
  return (struct timespec){0, 0};
}

struct timespec File::getLastStatusChange() {
  if (stat()) {
    return stat_.st_ctimespec;
  }
  return (struct timespec){0, 0};
}

struct timespec File::getCreation() {
  if (stat()) {
    return stat_.st_birthtimespec;
  }
  return (struct timespec){0, 0};
}

std::string File::getContent() {
  if (stat() && getType() == REG && isReadable()) {
    std::stringstream buffer;
    buffer << getIStream()->rdbuf();
    return buffer.str();
  }
  return "";
}

std::ifstream* File::getIStream() {
  if (stat() || stat_.st_ctimespec.tv_sec != last_time.tv_sec || stat_.st_ctimespec.tv_nsec != last_time.tv_nsec) {
    if (isReadable() && getType() == REG) {
      if (input_stream_ == NULL) {
        input_stream_ = new std::ifstream(path_);
      }
      return input_stream_;
    }
  } else if (input_stream_ != NULL) {
    input_stream_->close();
    delete input_stream_;
    input_stream_ = NULL;
  }
  return NULL;
}

std::ofstream* File::getOStream() {
  if (stat()) {
    if (isWritable() && getType() == REG) {
      if (output_stream_ == NULL) {
        output_stream_ = new std::ofstream(path_);
      }
      return output_stream_;
    }
  } else if (output_stream_ != NULL) {
    output_stream_->close();
    delete output_stream_;
    output_stream_ = NULL;
  }
  return NULL;
}