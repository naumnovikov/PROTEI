#ifndef APP_H
#define APP_H

#include "networkaddress.h"

#include <iostream>
#include <vector>

struct Device{
    NetworkAddress socket;
    std::vector<char> imei;
    std::vector<char> imsi;
    std::vector<float> location;
    std::string config;
    std::string nodes;
};

enum class Status{
    ACTIVE, NON_ACTIVE
};

void printStatus(Status status) noexcept;

enum class TypeOfProtocol{
    JSON, BINARY
};

void printTypeOfProtocol(TypeOfProtocol typeOfProtocol) noexcept;

enum class AppWorkingState{     // only for loop in interact()
    WORKING, NOT_WORKING
};

class App{
private:
    Device device;
    Status status{Status::NON_ACTIVE};
    TypeOfProtocol typeOfProtocol{TypeOfProtocol::JSON};
    AppWorkingState appWorkingState{AppWorkingState::WORKING};
public:
    std::vector<std::string> interpretateInputCommand(std::string command_buffer);
    void configurate(std::string json_filenameParam);
    void printMenu() noexcept;
    void ProcessACTIVE(const std::vector<std::string>& tokens);
    void ProcessMOVE(const std::vector<std::string>& tokens);
    void ProcessEXIT(const std::vector<std::string>& tokens);
    void ProcessPROTOCOL(const std::vector<std::string>& tokens);
    std::string inputCommand();
    void interact();

    Status getStatus() const noexcept {return status;}
    void setStatus(Status statusParam) noexcept {status = statusParam;}
    TypeOfProtocol getProtocol() const noexcept {return typeOfProtocol;}
    void setProtocol(TypeOfProtocol typeOfProtocolParam) noexcept {typeOfProtocol = typeOfProtocolParam;}
    void setAppWorkingState(AppWorkingState appWorkingStateParam) noexcept {appWorkingState = appWorkingStateParam;}
    std::vector<float>& getLocationForMoving() noexcept {return device.location;}


    // for tests
    const Device& getDevice() const noexcept {return device;}
    bool isWorking() const noexcept {return appWorkingState == AppWorkingState::WORKING ? true : false;}
};


#endif // APP_H
