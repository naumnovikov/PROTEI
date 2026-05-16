#ifndef SERVER_H
#define SERVER_H

#include <cstdint>

#include "jsonparser.h"
#include "workingstate.h"

class Server{
private:
    uint16_t port;
    std::vector<float> position;
    WorkingState serverWorkingState{WorkingState::WORKING};

    std::string inputPosition();
    std::vector<std::string> interpretateInput(std::string input_buffer);
    void processPositionInput(std::vector<std::string> tokens);
public:
    Server(){}

    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;

    void interact();

    bool isExitCommand(std::string firstToken);
    uint16_t getPort() noexcept {return port;}
    std::vector<float> getPosition() noexcept {return position;}
    void setPort(uint16_t portParam) noexcept {port = portParam;}
    void setPosition(std::vector<float> positionParam) noexcept {position = positionParam;}
};

#endif // SERVER_H