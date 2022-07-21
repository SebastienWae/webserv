#ifndef SERVER_H
#define SERVER_H
class server {
public:
  server();
  server& operator=(server const& rhs);
  server(server const& src);
  ~server();

private:
  int port;
  std::string server_names;
};
#endif