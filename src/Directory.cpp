#include "Directory.h"

<<<<<<< HEAD
std::string Directory::html(std::string const &path, std::string const &url) {
=======
std::string Directory::html(std::string const &path) {
>>>>>>>  Directory html
  struct stat sb;
  struct dirent *ent;
  DIR *dir;
  std::string str;
<<<<<<< HEAD
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
=======
  size_t n = path.length();
  char char_array[n + 1];
  std::string tmp
      = "<html> <head> <title></title></head><body><h1> Index of / path / to/ dir</h1><hr><table><tr><th> Name</"
        "th><th> Size</th><th> Last Modified</th></tr>";
  strcpy(char_array, path.c_str());
  str.insert(0, tmp);
  if ((dir = opendir(char_array)) != NULL) {
    while ((ent = readdir(dir)) != NULL) {
      stat(strcat(char_array, ent->d_name), &sb);
      str.insert(str.size(), "<tr>");
      str.insert(str.size(), "<td><a href='");
      str.insert(str.size(), ent->d_name);
      str.insert(str.size(), "'>");
>>>>>>>  Directory html
      str.insert(str.size(), ent->d_name);
      str.insert(str.size(), "</a></td><td>");
      str.insert(str.size(), std::to_string(sb.st_size));  // size
      str.insert(str.size(), "</td><td>");
      str.insert(str.size(), ctime(&sb.st_mtime));
      str.insert(str.size(), "</td>");
      str.insert(str.size(), "</tr>");
<<<<<<< HEAD
=======
      memset(char_array, 0, n);
      strcpy(char_array, path.c_str());
>>>>>>>  Directory html
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