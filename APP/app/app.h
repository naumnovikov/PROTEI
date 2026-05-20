#ifndef APP_H
#define APP_H

#include <iostream>

#include "networkaddress.h"
#include "jsonparser.h"
#include "workingstate.h"
#include "socketWorker.h"

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

class App{
private:
    Device device;
    Status status{Status::NON_ACTIVE};
    TypeOfProtocol typeOfProtocol{TypeOfProtocol::JSON};
    WorkingState appWorkingState{WorkingState::WORKING};
    SocketWorker socketWorker;

    void printMenu() const noexcept;
    std::string inputCommand();
    void processInteract(int sock);
    void ProcessACTIVE(const std::vector<std::string>& tokens);
    void ProcessMOVE(std::vector<std::string> tokens, int sock);
    void ProcessEXIT(const std::vector<std::string>& tokens);
    void ProcessPROTOCOL(const std::vector<std::string>& tokens);
    std::vector<std::string> interpretateInputCommand(std::string command_buffer);
    void configurate(std::string json_filenameParam);
public:
    void interact();

    Status getStatus() const noexcept {return status;}
    void setStatus(Status statusParam) noexcept {status = statusParam;}
    TypeOfProtocol getProtocol() const noexcept {return typeOfProtocol;}
    void setProtocol(TypeOfProtocol typeOfProtocolParam) noexcept {typeOfProtocol = typeOfProtocolParam;}
    void setAppWorkingState(WorkingState appWorkingStateParam) noexcept {appWorkingState = appWorkingStateParam;}
    std::vector<float>& getDeviceLocation() noexcept {return device.location;} 
    const Device& getDevice() const noexcept {return device;}
    void setDeviceSocket(NetworkAddress socketParam) noexcept {device.socket = socketParam;}
    void setDeviceIMEI(std::vector<char> imeiParam) noexcept {device.imei = imeiParam;}
    void setDeviceIMSI(std::vector<char> imsiParam) noexcept {device.imsi = imsiParam;}
    void setDeviceLocation(std::vector<float> locationParam) noexcept {device.location = locationParam;}
    void setDeviceConfig(std::string configParam) noexcept {device.config = configParam;}
    void setDeviceNodes(std::string nodesParam) noexcept {device.nodes = nodesParam;}
    bool isWorking() const noexcept {return appWorkingState == WorkingState::WORKING ? true : false;}
};


#endif // APP_H
