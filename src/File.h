#ifndef FILE_H
#define FILE_H

#include <sys/_types/_timespec.h>
#include <sys/stat.h>

#include <fstream>
#include <string>
#include <vector>

class File {
public:
  enum type { NONE, FIFO, CHR, DIR, BLK, REG, LNK, SOCK, WHT };

  File(std::string const& path);
  ~File();

  bool exist();

  bool isReadable();
  bool isWritable();
  bool isExecutable();

  enum type getType();

  struct timespec getLastAccess();
  struct timespec getLastModification();
  struct timespec getLastStatusChange();
  struct timespec getCreation();

  std::string getContent();
  std::string getMimeType();

  std::ifstream* getIStream();
  std::ofstream* getOStream();

private:
  std::string const path_;
  struct stat stat_;
  std::ifstream* input_stream_;
  std::ofstream* output_stream_;
  std::string error_;
  struct timespec last_time;

  bool stat();
};

#endif
