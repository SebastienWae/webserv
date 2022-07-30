#include "Server.h"

#include "HttpResponse.h"
#include "HttpResponseStatus.h"
#include "Log.h"

Server::Server(Config const& config) : config_(config), kq_(kqueue()) {
  std::vector<ServerConfig> const* servers_config = config_.getServerConfig();
  for (std::vector<ServerConfig>::const_iterator it = servers_config->begin(); it != servers_config->end(); ++it) {
    ports_.insert(it->getport());
  }
}

Server::~Server() {
  INFO("Stoping server");

  for (std::vector<int>::iterator it = ports_socket_.begin(); it != ports_socket_.end(); ++it) {
    closeConnection(*it);
  }

  for (std::map<int, Client*>::iterator it = clients_.begin(); it != clients_.end(); ++it) {
    closeConnection(it->first);
    delete it->second;
  }

  close(kq_);
}

void Server::start() {  // NOLINT
  INFO("Starting server");

  for (std::set<std::string>::const_iterator it = ports_.begin(); it != ports_.end(); ++it) {
    listenToPort(*it);
  }

  while (true) {
    timespec timeout = {1, 0};
    int n_events = kevent(kq_, NULL, 0, events_, KQ_SIZE, &timeout);  // NOLINT
    if (n_events == 0) {
      for (std::map<int, Client*>::iterator it = clients_.begin(); it != clients_.end(); ++it) {
        if (it->second->getTime() + TIMEOUT >= std::time(nullptr)) {
          timeoutClient(it->second);
        }
      }
    } else if (n_events < 0) {
      ERROR(std::strerror(errno));
    }

    for (int i = 0; i < n_events; ++i) {
      int socket = static_cast<int>(events_[i].ident);

      if (std::find(ports_socket_.begin(), ports_socket_.end(), socket) != ports_socket_.end()) {
        acceptConnection(socket);
      } else {
        Client* client = findClient(socket);
        try {
          if (client == NULL) {
            closeConnection(socket);
            ERROR("Client not found");
          } else if ((events_[i].flags & EV_EOF) != 0 || client->hasReplied()) {
            removeClient(client);
          } else if (events_[i].filter == EVFILT_READ) {
            if (client->getTime() + TIMEOUT >= std::time(nullptr)) {
              timeoutClient(client);
            } else {
              client->read(events_[i].data);
              if (!client->isReading()) {
                updateEvents(socket, EVFILT_READ, EV_DELETE);
                updateEvents(socket, EVFILT_WRITE, EV_ADD | EV_ENABLE);
                processRequest(client);
              }
            }
          } else if (events_[i].filter == EVFILT_WRITE) {
            client->send(events_[i].data);
          }
        } catch (std::exception& e) {
          if (client == NULL) {
            closeConnection(socket);
          } else {
            removeClient(client);
          }
        }
      }
    }
  }
}

// TODO
void Server::processRequest(Client* client) {}

void Server::timeoutClient(Client* client) {
  if (client->isReading()) {
    HttpResponse timeout_response(HttpResponseClientError::_408, NULL);
    client->setResponseData(timeout_response.getRaw());
    client->setRead();
    updateEvents(client->getSocket(), EVFILT_READ, EV_DELETE);
    updateEvents(client->getSocket(), EVFILT_WRITE, EV_ADD | EV_ENABLE);
    client->send(0);
  }
}

void Server::listenToPort(std::string const& port) {
  INFO("Trying to listen to port: " + port);

  struct addrinfo hints;
  memset((char*)&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = 0;

  struct addrinfo* address_info;
  int error = getaddrinfo(NULL, port.c_str(), &hints, &address_info);
  if (error != 0) {
    ERROR(gai_strerror(error));
    return;
  }
  int port_socket;
  struct addrinfo* tmp;
  for (tmp = address_info; tmp != NULL; tmp = tmp->ai_next) {
    port_socket = socket(tmp->ai_family, tmp->ai_socktype, tmp->ai_flags);
    if (port_socket < 0) {
      continue;
    }
    int y = 1;
    setsockopt(port_socket, SOL_SOCKET, SO_REUSEADDR, &y, sizeof(int));
    if (fcntl(port_socket, F_SETFL, O_NONBLOCK) == -1) {
      ERROR(std::strerror(errno));
      return;
    }
    if (bind(port_socket, tmp->ai_addr, tmp->ai_addrlen) < 0) {
      close(port_socket);
      ERROR(std::strerror(errno));
      continue;
    }
    break;
  }
  freeaddrinfo(address_info);
  if (tmp == NULL) {
    ERROR("Socket creation error");
    return;
  }
  if (listen(port_socket, KQ_SIZE) < 0) {
    ERROR(std::strerror(errno));
    return;
  }
  ports_socket_.push_back(port_socket);
  updateEvents(port_socket, EVFILT_READ, EV_ADD | EV_ENABLE);
  INFO("Listening to port: " + port);
}

void Server::closeConnection(int socket) {
  INFO("Connection closed");

  updateEvents(socket, EVFILT_READ, EV_DELETE);
  updateEvents(socket, EVFILT_WRITE, EV_DELETE);

  close(socket);
}

void Server::acceptConnection(int socket) {
  INFO("New connection");

  sockaddr_in client_addr;
  int addr_len = sizeof(client_addr);
  int connection_socket = accept(socket, (sockaddr*)&client_addr, (socklen_t*)&addr_len);
  if (connection_socket == INVALID_SOCKET) {
    return;
  }

  clients_[connection_socket] = new Client(connection_socket);

  fcntl(connection_socket, F_SETFL, O_NONBLOCK);

  updateEvents(connection_socket, EVFILT_READ, EV_ADD | EV_ENABLE);
}

void Server::updateEvents(int ident, short filter, u_short flags) {  // NOLINT
  timespec timeout = {1, 0};
  struct kevent kev;
  EV_SET(&kev, ident, filter, flags, 0, 0, &timeout);
  int n = kevent(kq_, &kev, 1, NULL, 0, 0);
  if (n == 0) {
    INFO("Kevent timeout");
  } else if (n < 0) {
    ERROR(std::strerror(errno));
  }
}

Client* Server::findClient(int socket) {
  std::map<int, Client*>::iterator client = clients_.find(socket);
  if (client != clients_.end()) {
    return client->second;
  }
  return NULL;
}

void Server::removeClient(Client* client) {
  int socket = client->getSocket();
  closeConnection(socket);
  delete client;
  clients_.erase(socket);
}