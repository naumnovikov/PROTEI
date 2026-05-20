#ifndef SERVER_H
#define SERVER_H

#include "jsonparser.h"
#include "workingstate.h"
#include "socketWorker.h"

using IPv4 = std::string;

class Server{
private:
    uint16_t port;
    IPv4 ip;
    std::vector<float> position;
    WorkingState serverWorkingState{WorkingState::WORKING};
    SocketWorker socketWorker;

    std::string inputPosition();
    std::vector<std::string> interpretateInput(std::string input_buffer);
    void processPositionInput(std::vector<std::string> tokens);
    bool isExitCommand(std::string firstToken);
    void processClients(int listenerForConnections);
public:
    Server(){}

    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;

    void interact();

    uint16_t getPort() const noexcept {return port;}
    std::vector<float> getPosition() const noexcept {return position;}
    IPv4 getIp() const noexcept {return ip;}
    void setPort(uint16_t portParam) noexcept {port = portParam;}
    void setPosition(std::vector<float> positionParam) noexcept {position = positionParam;}
    void setIp(IPv4 ipParam) noexcept {ip = ipParam;};
};

#endif // SERVER_H