#include "Server.h"

#include <_types/_uint8_t.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/signal.h>

#include <__nullptr>
#include <fstream>
#include <iostream>
#include <ostream>
#include <vector>

#include "CGI.h"
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
        if (std::time(nullptr) - (*it).second->getTime() >= TIMEOUT) {
          timeoutClient(it->second);
          break;
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
          } else if (std::time(nullptr) - client->getTime() >= TIMEOUT) {
            timeoutClient(client);
          } else if (events_[i].filter == EVFILT_READ) {
            client->read(events_[i].data, config_);
            if (!client->isReading()) {
              processRequest(client);
              updateEvents(client->getSocket(), EVFILT_READ, EV_DELETE);
              updateEvents(client->getSocket(), EVFILT_WRITE, EV_ADD | EV_ENABLE);
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
  ServerConfig const* server_config = config_.matchServerConfig(client->getRequest()->getHost());

  switch (client->getRequest()->getStatus()) {
    case HttpRequest::S_NONE: {
      response = new HttpResponse(HttpResponseServerError::_500, server_config);
      client->setReponse(response);
      return;
    }
    case HttpRequest::S_OK: {
      switch (client->getRequest()->getMethod()) {
        case Http::UNKNOWN: {
          response = new HttpResponse(HttpResponseClientError::_400, server_config);
          client->setReponse(response);
          return;
        }
        case Http::GET:
          getHandler(client, server_config);
          return;
        case Http::POST:
          postHandler(client, server_config);
          return;
        case Http::DELETE:
          deleteHandler(client, server_config);
          return;
      }
    }
    case HttpRequest::S_BAD_REQUEST: {
      response = new HttpResponse(HttpResponseClientError::_400, server_config);
      client->setReponse(response);
      return;
    }
    case HttpRequest::S_NOT_IMPLEMENTED: {
      response = new HttpResponse(HttpResponseServerError::_501, server_config);
      client->setReponse(response);
      return;
    }
    case HttpRequest::S_HTTP_VERSION_NOT_SUPPORTED: {
      response = new HttpResponse(HttpResponseServerError::_505, server_config);
      client->setReponse(response);
      return;
    }
    case HttpRequest::S_EXPECTATION_FAILED: {
      response = new HttpResponse(HttpResponseClientError::_417, server_config);
      client->setReponse(response);
      return;
    }
    case HttpRequest::S_LENGTH_REQUIRED: {
      response = new HttpResponse(HttpResponseClientError::_411, server_config);
      client->setReponse(response);
      return;
    }
    case HttpRequest::S_REQUEST_ENTITY_TOO_LARGE: {
      response = new HttpResponse(HttpResponseClientError::_413, server_config);
      client->setReponse(response);
      return;
    }
    case HttpRequest::S_CONTINUE: {
      response = new HttpResponse(HttpResponseServerError::_500, server_config);
      client->setReponse(response);
      return;
    }
  }
}

void Server::getHandler(Client* client, ServerConfig const* server_config) {
  HttpRequest const* req = client->getRequest();
  Uri const* uri = req->getUri();
  Route const* route = server_config->matchRoute(uri);

  HttpResponse* response;
  if (route->isAllowedMethod(Http::GET)) {
    if (route->isRedirection()) {
      response = new HttpResponse(route->getRedirection().first, route->getRedirection().second->getRaw());
    } else {
      try {
        File* target = route->matchCGI(uri);
        if (target == nullptr) {
          target = route->matchFile(uri);
          if (target->getType() == File::DI) {
            if (route->isDirectoryListing()) {
              response = new HttpResponse(HttpResponseSuccess::_200, target->getListing(uri->getDecodedPath()),
                                          "text/html", server_config);
            } else {
              delete target;
              target = route->getDirecoryPage();
              if (target == nullptr || !target->exist() || !target->isReadable() || !(target->getType() == File::REG)) {
                response = new HttpResponse(HttpResponseClientError::_403, server_config);
              } else {
                response
                    = new HttpResponse(HttpResponseSuccess::_200, target->getContent(), "text/html", server_config);
              }
            }
          } else {
            response = new HttpResponse(HttpResponseSuccess::_200, target, server_config);
          }
        } else {
          CGI script(client, server_config, target, "GET");
          script.execute();
          return;
        }
      } catch (Route::NotFoundException) {
        response = new HttpResponse(HttpResponseClientError::_404, server_config);
      } catch (Route::ForbiddenException) {
        response = new HttpResponse(HttpResponseClientError::_403, server_config);
      }
    }
  } else {
    response = new HttpResponse(HttpResponseClientError::_405, server_config);
  }
  client->setReponse(response);
}

void Server::postHandler(Client* client, ServerConfig const* server_config) {
  HttpRequest const* req = client->getRequest();
  Uri const* uri = req->getUri();
  Route const* route = server_config->matchRoute(uri);

  HttpResponse* response;
  if (route->isAllowedMethod(Http::POST)) {
    if (route->isRedirection()) {
      response = new HttpResponse(route->getRedirection().first, route->getRedirection().second->getRaw());
    } else {
      try {
        File* target = route->matchCGI(uri);
        if (target == nullptr) {
          std::map<std::string, std::string> headers = req->getHeaders();
          std::map<std::string, std::string>::const_iterator content_type = headers.find("content-type");
          if (content_type != headers.end() && (content_type->second.compare(0, 9, "multipart") == 0)) {
            response = new HttpResponse(HttpResponseClientError::_415, server_config);
          } else {
            target = route->matchFile(uri);
            if (target->getType() == File::DI) {
              if (route->isDirectoryListing()) {
                response = new HttpResponse(HttpResponseSuccess::_200, target->getListing(uri->getDecodedPath()),
                                            "text/html", server_config);
              } else {
                delete target;
                target = route->getDirecoryPage();
                if (target == nullptr || !target->exist() || !target->isReadable()
                    || !(target->getType() == File::REG)) {
                  response = new HttpResponse(HttpResponseClientError::_403, server_config);
                } else {
                  response
                      = new HttpResponse(HttpResponseSuccess::_200, target->getContent(), "text/html", server_config);
                }
              }
            } else {
              response = new HttpResponse(HttpResponseSuccess::_200, target, server_config);
            }
          }
        } else {
          CGI script(client, server_config, target, "POST");
          script.execute();
          return;
        }
      } catch (Route::NotFoundException) {
        if (req->isFileUpload()) {
          File* target = route->getUploadStore();
          if (target != nullptr && target->exist() && target->isWritable() && target->getType() == File::DI) {
            File* upload = route->matchFileUpload(uri);
            if (upload->exist()) {
              response = new HttpResponse(HttpResponseClientError::_409, server_config);
            } else {
              std::vector<uint8_t> file = req->getBody();
              upload->getOStream()->write(reinterpret_cast<char*>(&file[0]), file.size());  // NOLINT
              response = new HttpResponse(HttpResponseSuccess::_201, server_config);
            }
            delete upload;
          } else {
            response = new HttpResponse(HttpResponseClientError::_403, server_config);
          }
        } else {
          response = new HttpResponse(HttpResponseClientError::_404, server_config);
        }
      } catch (Route::ForbiddenException) {
        response = new HttpResponse(HttpResponseClientError::_403, server_config);
      }
    }
  } else {
    response = new HttpResponse(HttpResponseClientError::_405, server_config);
  }
  client->setReponse(response);
}

void Server::deleteHandler(Client* client, ServerConfig const* server_config) {
  HttpRequest const* req = client->getRequest();
  Uri const* uri = req->getUri();
  Route const* route = server_config->matchRoute(uri);

  HttpResponse* response;
  if (route->isAllowedMethod(Http::DELETE)) {
    if (route->isRedirection()) {
      response = new HttpResponse(route->getRedirection().first, route->getRedirection().second->getRaw());
    } else {
      try {
        File* target = route->matchCGI(uri);
        if (target == nullptr) {
          target = route->matchFile(uri);
        }
        if (target->isWritable()) {
          if (target->getType() == File::DI) {
            if (remove(target->getPath().c_str()) != 0) {
              response = new HttpResponse(HttpResponseClientError::_403, server_config);
            } else {
              response = new HttpResponse(HttpResponseSuccess::_200, "Directory deleted.", "text/html", server_config);
            }
          } else {
            if (remove(target->getPath().c_str()) != 0) {
              response = new HttpResponse(HttpResponseServerError::_500, server_config);
            } else {
              response = new HttpResponse(HttpResponseSuccess::_200, "File deleted.", "text/html", server_config);
            }
          }
        } else {
          throw Route::ForbiddenException();
        }
      } catch (Route::NotFoundException) {
        response = new HttpResponse(HttpResponseClientError::_404, server_config);
      } catch (Route::ForbiddenException) {
        response = new HttpResponse(HttpResponseClientError::_403, server_config);
      }
    }
  } else {
    response = new HttpResponse(HttpResponseClientError::_405, server_config);
  }
  client->setReponse(response);
  updateEvents(client->getSocket(), EVFILT_WRITE, EV_ADD | EV_ENABLE);
}

void Server::timeoutClient(Client* client) {
  INFO("Timeout client");
  if (client->isReading()) {
    HttpResponse* timeout_response;
    if (client->getRequest() != NULL) {
      ServerConfig const* server_config = config_.matchServerConfig(client->getRequest()->getHost());
      timeout_response = new HttpResponse(HttpResponseClientError::_408, server_config);
    } else {
      timeout_response = new HttpResponse(HttpResponseClientError::_408, nullptr);
    }
    client->setReponse(timeout_response);
    client->setRead();
  } else {
    removeClient(client);
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

  clients_[connection_socket] = new Client(connection_socket, client_addr.sin_addr);

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