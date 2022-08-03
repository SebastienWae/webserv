#include "Server.h"

#include <iostream>
#include <vector>

#include "Cgi.h"
#include "Directory.h"
#include "File.h"
#include "Http.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "HttpResponseStatus.h"
#include "Log.h"
#include "ServerConfig.h"
Server::Server(Config const& config) : config_(config), kq_(kqueue()) {}

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

  std::set<std::string> ports = config_.getPorts();
  for (std::set<std::string>::const_iterator it = ports.begin(); it != ports.end(); ++it) {
    listenToPort(*it);
  }

  while (true) {
    timespec timeout = {1, 0};
    int n_events = kevent(kq_, NULL, 0, events_, KQ_SIZE, &timeout);  // NOLINT
    if (n_events == 0) {
      for (std::map<int, Client*>::iterator it = clients_.begin(); it != clients_.end(); ++it) {
        if (it->second->getTime() + TIMEOUT <= std::time(nullptr)) {
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
            // TODO : differenciate clients events and cgi events
            if (client->getTime() + TIMEOUT <= std::time(nullptr)) {
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
            if (!client->hasReplied()) {
              client->send(events_[i].data);
            } else {
              removeClient(client);
            }
          }
        } catch (std::exception& e) {
          if (client == NULL) {
            closeConnection(socket);
          } else {
            removeClient(client);
          }
          std::cerr << e.what() << std::endl;
        }
      }
    }
  }
}

void Server::processRequest(Client* client) {
  HttpResponse* response;
  ServerConfig const* server_config = config_.matchServerConfig(client->getRequest());

  switch (client->getRequest()->getStatus()) {
    case HttpRequest::S_NONE: {
      response = new HttpResponse(HttpResponseServerError::_500, server_config);
      client->setResponseData(response->getRaw());
      delete response;
      return;
    }
    case HttpRequest::S_OK: {
      switch (client->getRequest()->getMethod()) {
        case Http::UNKNOWN: {
          response = new HttpResponse(HttpResponseClientError::_400, server_config);
          client->setResponseData(response->getRaw());
          delete response;
          return;
        }
        case Http::GET:
          getHandler(client, server_config);
          return;
        case Http::HEAD:
          headHandler(client, server_config);
          return;
        case Http::POST:
          postHandler(client, server_config);
          return;
        case Http::DELETE:
          deleteHandler(client, server_config);
          return;
      }
    }
    case HttpRequest::S_CONTINUE: {  // this should not be possible
      response = new HttpResponse(HttpResponseServerError::_500, server_config);
      client->setResponseData(response->getRaw());
      delete response;
      return;
    }
    case HttpRequest::S_BAD_REQUEST: {
      response = new HttpResponse(HttpResponseClientError::_400, server_config);
      client->setResponseData(response->getRaw());
      delete response;
      return;
    }
    case HttpRequest::S_NOT_IMPLEMENTED: {
      response = new HttpResponse(HttpResponseServerError::_501, server_config);
      client->setResponseData(response->getRaw());
      delete response;
      return;
    }
    case HttpRequest::S_HTTP_VERSION_NOT_SUPPORTED: {
      response = new HttpResponse(HttpResponseServerError::_505, server_config);
      client->setResponseData(response->getRaw());
      delete response;
      return;
    }
    case HttpRequest::S_EXPECTATION_FAILED: {
      response = new HttpResponse(HttpResponseClientError::_417, server_config);
      client->setResponseData(response->getRaw());
      delete response;
      return;
    }
  }
}

void Server::getHandler(Client* client, ServerConfig const* server_config) {
  HttpRequest const* req = client->getRequest();
  Route const* route = server_config->matchRoute(req->getUri());

  HttpResponse* response;
  if (route->isAllowedMethod(Http::GET)) {
    if (route->isRedirection()) {
      response = new HttpResponse(route->getRedirection().first, route->getRedirection().second->getRaw());
    } else {
      // TODO : cgi matching
      Cgi cgitest(client, server_config, "GET");
      cgitest.executeCgi(kq_, client);
      response = new HttpResponse(HttpResponseSuccess::_200, server_config);
      // File* file = route->matchFile(req->getUri().getPath());
      // if (file->isReadable()) {
      //   if (file->getType() == File::REG) {
      //     response = new HttpResponse(HttpResponseSuccess::_200, file->getContent(), "text/html", server_config);
      //   } else if (file->getType() == File::DIR) {
      //     response
      //         = new HttpResponse(HttpResponseSuccess::_200,
      //                            Directory::html(file->getPath(), server_config->getHost() +
      //                            req->getUri().getPath()), "text/html", server_config);
      //   } else {
      //     // wrong type
      //     response = new HttpResponse(HttpResponseSuccess::_200, server_config);
      //   }
      // } else {
      //   // wrong perm
      //   response = new HttpResponse(HttpResponseSuccess::_200, server_config);
      // }
    }
  } else {
    response = new HttpResponse(HttpResponseClientError::_405, server_config);
  }

  client->setResponseData(response->getRaw());
  delete response;
}

// TODO
void Server::headHandler(Client* client, ServerConfig const* server_config) {
  HttpResponse response(HttpResponseSuccess::_200, server_config);
  client->setResponseData(response.getRaw());
}

// TODO
void Server::postHandler(Client* client, ServerConfig const* server_config) {
  HttpResponse response(HttpResponseSuccess::_200, server_config);
  client->setResponseData(response.getRaw());
}

// TODO
void Server::deleteHandler(Client* client, ServerConfig const* server_config) {
  HttpResponse response(HttpResponseSuccess::_200, server_config);
  client->setResponseData(response.getRaw());
}

void Server::timeoutClient(Client* client) {
  INFO("Timeout client");
  if (client->isReading()) {
    if (client->getRequest() != NULL) {
      ServerConfig const* server_config = config_.matchServerConfig(client->getRequest());
      HttpResponse timeout_response(HttpResponseClientError::_408, server_config);

      client->setResponseData(timeout_response.getRaw());
      client->setRead();
    }

    updateEvents(client->getSocket(), EVFILT_READ, EV_DELETE);
    updateEvents(client->getSocket(), EVFILT_WRITE, EV_ADD | EV_ENABLE);
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
  // TODO : add IP to client object
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