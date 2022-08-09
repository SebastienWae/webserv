#ifndef FILE_H
#define FILE_H

#include <sys/stat.h>

#include <string>
#include <vector>

class File {
public:
  enum type { NONE, FIFO, CHR, DI, BLK, REG, LNK, SOCK, WHT };

  File(std::string const& path);
  ~File();

  std::string const& getPath() const;

  bool exist();

  bool isReadable();
  bool isWritable();
  bool isExecutable();

  std::size_t getSize();

  enum type getType();

  struct timespec getLastAccess();
  struct timespec getLastModification();
  struct timespec getLastStatusChange();
  struct timespec getCreation();

  std::string getContent();
  std::string getMimeType();

  std::string getListing(std::string const& url);

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
