#ifndef DIRECTORY_H
#define DIRECTORY_H
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <exception>
#include <filesystem>
#include <iostream>
#include <string>

class Directory {
public:
<<<<<<< HEAD
  std::string static html(std::string const &path, std::string const &url);
=======
  std::string static html(std::string const &path);
>>>>>>>  Directory html
  class PathException : public std::exception {
  public:
    virtual const char *what(void) const throw();
  };
};

#endif