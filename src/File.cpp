#include "File.h"

#include <dirent.h>
#include <sys/_types/_timespec.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstddef>
#include <fstream>
#include <iomanip>
#include <sstream>

std::vector<std::pair<std::string const, std::string const> > initMimeFileType() {
  std::vector<std::pair<std::string const, std::string const> > mimeTypes;
  mimeTypes.push_back(std::pair<std::string const, std::string>(".aac", "audio/aac"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".avi ", "video/x-msvideo"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".bin", "application/octet-stream"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".bmp", "image/bmp"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".bz", "application/x-bzip"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".bz2", "application/x-bzip2"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".csh", "application/x-csh"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".css", "text/css"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".csv", "text/csv"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".doc", "application/msword"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(
      ".docx ", "application/vnd.openxmlformats-officedocument.wordprocessingml.document"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".gif", "image/gif"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".htm", "text/html"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".html", "text/html"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".ico", "image/x-icon"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".jar", "application/java-archive"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".jpeg", "image/jpeg"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".jpg", "image/jpeg"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".js", "application/javascript"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".json", "application/json"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".mid", "audio/midi"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".midi", "audio/midi"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".mpeg", "video/mpeg"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".mp3", "audio/mpeg"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".mp4", "video/mp4"));
  mimeTypes.push_back(
      std::pair<std::string const, std::string>(".odp ", "application/vnd.oasis.opendocument.presentation"));
  mimeTypes.push_back(
      std::pair<std::string const, std::string>(".ods", "application/vnd.oasis.opendocument.spreadsheet"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".odt", "application/vnd.oasis.opendocument.text"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".oga", "audio/ogg"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".ogv", "video/ogg"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".ogx", "application/ogg"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".otf", "font/otf"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".png", "image/png"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".pdf", "application/pdf"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".ppt", "application/vnd.ms-powerpoint"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(
      ".pptx", "application/vnd.openxmlformats-officedocument.presentationml.presentation"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".rar", "application/x-rar-compressed"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".rtf", "application/rtf"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".sh", "application/x-sh"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".svg", "image/svg+xml"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".swf", "application/x-shockwave-flash"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".tar", "application/x-tar"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".tif", "image/tiff"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".tiff", "image/tiff"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".ts", "application/typescript"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".ttf", "font/ttf"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".txt", "text/plain"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".vsd", "application/vnd.visio"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".wav", "audio/x-wav"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".weba", "audio/webm"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".webm", "video/webm"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".webp ", "image/webp"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".woff", "font/woff"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".woff2", "font/woff2"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".xhtml", "application/xhtml+xml"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".xls", "application/vnd.ms-excel"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(
      ".xlsx", "application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".xml", "application/xml"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".xul", "application/vnd.mozilla.xul+xml"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".zip", "application/zip"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".3gp", "video/3gpp"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".3g2", "video/3gpp2"));
  mimeTypes.push_back(std::pair<std::string const, std::string>(".7z", "	application/x-7z-compressed"));
  return (mimeTypes);
}

std::vector<std::pair<std::string const, std::string const> > mimeTypes = initMimeFileType();

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

std::size_t File::getSize() {
  if (stat()) {
    return stat_.st_size;
  }
  return 0;
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
      return DI;
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
    getIStream()->seekg(0);
    std::stringstream buffer;
    buffer << getIStream()->rdbuf();
    return buffer.str();
  }
  return "";
}

std::string File::getMimeType() {
  std::string::size_type spot = path_.find_last_of('.');
  if (spot != std::string::npos) {
    std::string ext = path_.substr(spot, path_.length());
    for (std::vector<std::pair<std::string const, std::string const> >::iterator it = mimeTypes.begin();
         it != mimeTypes.end(); it++) {
      if (ext == it->first) {
        return (it->second);
      }
    }
  }
  return ("application/octet-stream");
}

std::ifstream* File::getIStream() {
  if (stat() || stat_.st_ctimespec.tv_sec != last_time.tv_sec || stat_.st_ctimespec.tv_nsec != last_time.tv_nsec) {
    if (isReadable() && getType() == REG) {
      if (input_stream_ == NULL) {
        input_stream_ = new std::ifstream(path_, std::ios::binary);
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
  if (output_stream_ == NULL) {
    output_stream_ = new std::ofstream(path_, std::ios::binary);
  }
  return output_stream_;
}

std::string File::getListing(std::string const& url) {
  std::string html;
  struct stat sb;
  struct dirent* ent;

  if (stat()) {
    if (getType() == DI && isReadable()) {
      html = "<html><head><base href='" + url + "/'><title>" + path_
             + "</title><style>th {text-align: left;} table {width: 100%;}</style></head><body><h1> Index of " + path_
             + "</h1><hr><table><tr><th>Name</th><th>Size</th><th>Last Modified</th></tr>";
      DIR* dir = opendir(path_.c_str());
      while ((ent = readdir(dir)) != NULL) {
        ::stat((path_ + "/" + ent->d_name).c_str(), &sb);
        std::ostringstream size;
        if (sb.st_size > 1000000000) {
          size << std::setprecision(3) << sb.st_size * 0.000000001 << " GB";
        } else if (sb.st_size > 1000000) {
          size << std::setprecision(3) << sb.st_size * 0.000001 << " MB";
        } else if (sb.st_size > 1000) {
          size << std::setprecision(3) << sb.st_size * 0.001 << " kB";
        } else {
          size << sb.st_size << " B";
        }
        html += "<tr>";
        html += "<td><a href=\"";
        html += ent->d_name;
        html += "\">";
        html += ent->d_name;
        html += "</a></td><td>";
        html += size.str();
        html += "</td><td>";
        html += ctime(&sb.st_mtime);
        html += "</td>";
        html += "</tr>";
      }
      closedir(dir);
      html += "</table> <hr></body></html> ";
    }
  }
  return html;
}