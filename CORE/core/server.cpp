#include "server.h"

#include <ctype.h>
#include <spdlog/spdlog.h>

#include <cmath>
#include <iostream>
#include <limits>
#include <sstream>

#define CONNECTTION_TIMEOUT_SEC 240
#define CLIENT_RESPONCE_TIMEOUT_SEC 120
#define MIN_REST_LEN_BUFFER 0
#define MAX_REST_LEN_BUFFER 65536
#define CONNECTIONS_QUEUE_SIZE_LIMIT 5

Server::Server(Server&& other) noexcept
    : port(other.port), position(other.position) {
  other.port = 0;
  other.position = {0, 0, 0};
  spdlog::info("Servers' copy-constuctor used");
}

Server& Server::operator=(Server&& other) noexcept {
  if (this != &other) {
    port = other.port;
    position = other.position;
    other.port = 0;
    other.position = {0, 0, 0};
  }
  spdlog::info("Servers' move-operator used");
  return *this;
}

std::string Server::inputPosition() {
  std::cout << ">> ";
  std::string input_buffer;
  if (!std::getline(std::cin, input_buffer)) [[unlikely]] {
    serverWorkingState = WorkingState::NOT_WORKING;
    spdlog::error("Input stream closed or error");
    throw std::runtime_error("Input stream closed or error");
  }
  return input_buffer;
}

std::vector<std::string> Server::interpretateInput(std::string input_buffer) {
  std::vector<std::string> tokens;
  std::istringstream iss(std::move(input_buffer));
  std::string token;
  while (iss >> token) {
    if (!token.empty()) [[likely]] {
      tokens.push_back(std::move(token));
    }
  }
  return tokens;
}

bool Server::isExitCommand(std::string firstToken) {
  for (char& ch : firstToken) {
    ch = std::toupper(ch);
  }

  if (firstToken == "EXIT") {
    return true;
  }
  return false;
}

void Server::processPositionInput(std::vector<std::string> tokens) {
  std::size_t tokens_quantity{tokens.size()};
  if (tokens_quantity == 0) {
    throw std::invalid_argument("Empty input");
  }

  std::string firstToken{tokens.at(0)};
  if (isExitCommand(std::move(firstToken))) {
    serverWorkingState = WorkingState::NOT_WORKING;
    spdlog::info("Exiting...");
    return;
  }

  if (tokens_quantity != 3) [[unlikely]] {
    throw std::invalid_argument("Input must have 3 arguments");
  }

  std::vector<float> new_position;
  try {
    for (std::size_t i{0}; i < tokens_quantity; ++i) {
      new_position.push_back(std::stof(tokens.at(i)));
    }
  } catch (...) {
    spdlog::error("MOVE: invalid number format");
    throw std::invalid_argument("Invalid number format");
  }

  float distance{static_cast<float>(
      std::sqrt(std::pow(std::abs(position.at(0) - new_position.at(0)), 2) +
                std::pow(std::abs(position.at(1) - new_position.at(1)), 2) +
                std::pow(std::abs(position.at(2) - new_position.at(2)), 2)))};
  spdlog::info("Distance is {}", distance);
}

void Server::processClient(int client_sock, ThreadPool& pool,
                           const char* client_ip, uint16_t client_port) {
  struct timeval tv;
  tv.tv_sec = CLIENT_RESPONCE_TIMEOUT_SEC;
  tv.tv_usec = 0;
  if (setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
      [[unlikely]] {
    spdlog::warn("[{}:{}] Failed to set recv timeout", client_ip, client_port);
  }

  spdlog::info("[{}:{}] Client connected", client_ip, client_port);

  bool client_ok{true};
  while (client_ok && serverWorkingState == WorkingState::WORKING &&
         !pool.isTerminating()) {
    uint8_t len_buf[size_len];
    if (!socketWorker.recv_full(client_sock, len_buf, 4)) {
      client_ok = false;
      break;
    }
    uint32_t rest_len{socketWorker.decodeUint32FromBEBytes(len_buf)};
    if (rest_len == MIN_REST_LEN_BUFFER || rest_len > MAX_REST_LEN_BUFFER) {
      spdlog::error("[{}:{}] Invalid rest_len {}", client_ip, client_port,
                    rest_len);
      break;
    }
    if (!socketWorker.sendDistance(client_sock, rest_len, client_ok, position,
                                   client_ip, client_port)) {
      break;
    }
  }
  close(client_sock);
  spdlog::info("[{}:{}] Client disconnected", client_ip, client_port);
}

void Server::processClients(int listenerForConnections, ThreadPool& pool) {
  while (serverWorkingState == WorkingState::WORKING) {
    fd_set readfds;
    struct timeval tv_for_connections;

    FD_ZERO(&readfds);
    FD_SET(listenerForConnections, &readfds);

    tv_for_connections.tv_sec = CONNECTTION_TIMEOUT_SEC;
    tv_for_connections.tv_usec = 0;

    int activity{select(listenerForConnections + 1, &readfds, NULL, NULL,
                        &tv_for_connections)};

    if (activity < 0) {
      spdlog::error("Listening soket error");
      return;
    } else if (activity == 0) {
      spdlog::warn("Zero connnections to listening soket");
      return;
    }

    struct sockaddr_in client_addr;
    socklen_t addr_size{sizeof(client_addr)};

    int client_sock{accept(listenerForConnections,
                           (struct sockaddr*)&client_addr, &addr_size)};
    if (client_sock < 0) [[unlikely]] {
      spdlog::error("Accept error: {}", strerror(errno));
      continue;
    }
    char* client_ip{inet_ntoa(client_addr.sin_addr)};
    uint16_t client_port{ntohs(client_addr.sin_port)};
    pool.queueJob([this, client_sock, &pool, client_ip, client_port]() {
      processClient(client_sock, pool, client_ip, client_port);
    });
  }
}

void Server::interact() {
  spdlog::info("Configuring from serverconfig.json.");
  std::string json_filename{"serverconfig.json"};
  JSONParser parser;
  try {
    parser.configurateServer(std::move(json_filename), *this);
  } catch (const std::invalid_argument& e) {
    spdlog::error("Configuration error: {}", e.what());
    return;
  } catch (const std::out_of_range& e) {
    spdlog::error("Configuration error: {}", e.what());
    return;
  } catch (...) {
    spdlog::error("Configuration error.");
    return;
  }

  int listenerForConnections{socket(AF_INET, SOCK_STREAM, 0)};
  if (listenerForConnections < 0) [[unlikely]] {
    spdlog::error("listenerForConnections-socket creation error");
    return;
  }

  if (socketWorker.bindListenerForConnections(
          port, ip.c_str(), listenerForConnections) < 0) [[unlikely]] {
    spdlog::error("Bind error");
    close(listenerForConnections);
    return;
  }

  if (listen(listenerForConnections, CONNECTIONS_QUEUE_SIZE_LIMIT) < 0)
      [[unlikely]] {
    spdlog::error("Listen error");
    close(listenerForConnections);
    return;
  }

  spdlog::info("Server listening on {}:{}", getIp(), getPort());

  ThreadPool pool;
  pool.start();

  try {
    processClients(listenerForConnections, pool);
  } catch (...) {
    spdlog::error("Unknown exception in processClients");
  }

  pool.stop();
  close(listenerForConnections);
  spdlog::info("Server stopped");
}
