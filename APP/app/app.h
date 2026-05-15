#ifndef APP_H
#define APP_H

#include "networkaddress.h"

#include <iostream>
#include <vector>
#include <nlohmann/json.hpp>
#include <string_view>

using json = nlohmann::json;

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

    bool validateJSONOnRequiredFileds(const json& json_file) noexcept;
    bool isIPValid(std::string_view ip) noexcept;
    bool isIMEIValid(const std::vector<char>& imei) noexcept;
    bool isIMSIValid(const std::vector<char>& imsi) noexcept;
    bool isLocationValid(const std::vector<float>& location) noexcept;
    void setValues(const json& json_data);
    void printMenu() noexcept;
    std::string inputCommand();
public:
    std::vector<std::string> interpretateInputCommand(std::string command_buffer);
    void ProcessACTIVE(const std::vector<std::string>& tokens);
    void ProcessMOVE(const std::vector<std::string>& tokens);
    void ProcessEXIT(const std::vector<std::string>& tokens);
    void ProcessPROTOCOL(const std::vector<std::string>& tokens);
    void configurate(std::string json_filenameParam);
    void interact();


    Status getStatus() const noexcept {return status;}
    void setStatus(Status statusParam) noexcept {status = statusParam;}
    TypeOfProtocol getProtocol() const noexcept {return typeOfProtocol;}
    void setProtocol(TypeOfProtocol typeOfProtocolParam) noexcept {typeOfProtocol = typeOfProtocolParam;}
    void setAppWorkingState(AppWorkingState appWorkingStateParam) noexcept {appWorkingState = appWorkingStateParam;}
    std::vector<float>& getLocationForMoving() noexcept {return device.location;}

    const Device& getDevice() const noexcept {return device;}
    bool isWorking() const noexcept {return appWorkingState == AppWorkingState::WORKING ? true : false;}
};


#endif // APP_H
