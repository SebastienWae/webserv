#include "Directory.h"

std::string Directory::html(std::string const &path, std::string const &url) {
  struct stat sb;
  struct dirent *ent;
  DIR *dir;
  std::string str;
  std::string tmp
      = "<html> <head> <base href='"
      + url
      + "/' ><title></title></head><body><h1> Index of / path / to/ dir</h1><hr><table><tr><th> Name</"
        "th><th> Size</th><th> Last Modified</th></tr>";
  str.insert(0, tmp);
  if ((dir = opendir(path.c_str())) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      stat(path.c_str(), &sb);
      str.insert(str.size(), "<tr>");
      str.insert(str.size(), "<td><a href=\"");
      str.insert(str.size(), ent->d_name);
      str.insert(str.size(), "\">");
      str.insert(str.size(), ent->d_name);
      str.insert(str.size(), "</a></td><td>");
      str.insert(str.size(), std::to_string(sb.st_size));  // size
      str.insert(str.size(), "</td><td>");
      str.insert(str.size(), ctime(&sb.st_mtime));
      str.insert(str.size(), "</td>");
      str.insert(str.size(), "</tr>");
    }
    tmp = "</table> <hr></body></html> ";
    str.insert(str.size(), tmp);
    closedir(dir);
  } else {
    throw Directory::PathException();
  }
  return (str);
}

const char *Directory::PathException::what() const throw() { return ("Exception : Bad Path"); }