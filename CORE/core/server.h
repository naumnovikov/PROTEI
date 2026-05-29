#ifndef SERVER_H
#define SERVER_H

#include "jsonparser.h"
#include "socketWorker.h"
#include "threadpool.h"
#include "workingstate.h"

using IPv4 = std::string;
using position_vector = std::vector<float>

class Server {
 private:
  uint16_t port;
  IPv4 ip;
  position_vector position;
  std::atomic<WorkingState> serverWorkingState{WorkingState::WORKING};
  SocketWorker socketWorker;

  std::string inputPosition();
  std::vector<std::string> interpretateInput(std::string input_buffer);
  void processPositionInput(std::vector<std::string> tokens);
  bool isExitCommand(std::string firstToken);
  void processClient(int client_sock, ThreadPool& pool, const char* client_ip,
                     uint16_t client_port);
  void processClients(int listenerForConnections, ThreadPool& pool);

 public:
  Server() {}

  Server(Server&& other) noexcept;
  Server& operator=(Server&& other) noexcept;

  void interact();

  inline uint16_t getPort() const noexcept { return port; }
  inline position_vector getPosition() const noexcept { return position; }
  inline IPv4 getIp() const noexcept { return ip; }
  inline void setPort(uint16_t portParam) noexcept { port = portParam; }
  inline void setPosition(position_vector positionParam) noexcept {
    position = positionParam;
  }
  inline void setIp(IPv4 ipParam) noexcept { ip = ipParam; };
};

#endif  // SERVER_H