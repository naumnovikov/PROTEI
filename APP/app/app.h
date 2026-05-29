#ifndef APP_H
#define APP_H

#include <iostream>
#include <utility>

#include "jsonparser.h"
#include "networkaddress.h"
#include "socketWorker.h"
#include "workingstate.h"

struct Device {
  NetworkAddress serverAddress;
  std::vector<char> imei;
  std::vector<char> imsi;
  std::vector<float> location;
  std::string config;
  std::string nodes;
  std::string ip;
};

enum class Status { ACTIVE, NON_ACTIVE };

void printStatus(Status status) noexcept;

enum class TypeOfProtocol { JSON, BINARY };

void printTypeOfProtocol(TypeOfProtocol typeOfProtocol) noexcept;

class App {
 private:
  Device device;
  Status status{Status::NON_ACTIVE};
  TypeOfProtocol typeOfProtocol{TypeOfProtocol::JSON};
  WorkingState appWorkingState{WorkingState::WORKING};
  SocketWorker socketWorker;

  void printMenu() const noexcept;
  std::string inputCommand();
  void processInteract(int sock);

  //ProcessACTIVE, ProcessMOVE, ProcessEXIT, ProcessPROTOCOL
  //get vector not by & because in processInteract() I use std::move()
  //for tokens as it's not used later there
  void ProcessACTIVE(std::vector<std::string> tokens);
  void ProcessMOVE(std::vector<std::string> tokens, int sock);
  void ProcessEXIT(std::vector<std::string> tokens);
  void ProcessPROTOCOL(std::vector<std::string> tokens);

  std::vector<std::string> interpretateInputCommand(std::string command_buffer);
  void configurate(std::string json_filenameParam);
  void turnStringIntoUpper(const std::string& str);
 public:
  void interact();

  inline Status getStatus() const noexcept { return status; }
  inline void setStatus(Status statusParam) noexcept { status = statusParam; }
  inline TypeOfProtocol getProtocol() const noexcept { return typeOfProtocol; }
  inline void setProtocol(TypeOfProtocol typeOfProtocolParam) noexcept {
    typeOfProtocol = typeOfProtocolParam;
  }
  inline void setAppWorkingState(WorkingState appWorkingStateParam) noexcept {
    appWorkingState = appWorkingStateParam;
  }
  inline std::vector<float>& getDeviceLocationForFilling() noexcept { return device.location; }
  inline const Device& getDevice() const noexcept { return device; }

  //setters are using copy because when we execute them
  //we use std::move() in inputParam so it calls default move-constructor
  inline void setDeviceServerAddress(NetworkAddress serverAddressParam) noexcept {
    device.serverAddress = std::move(serverAddressParam);
  }
  inline void setDeviceIMEI(std::vector<char> imeiParam) noexcept {
    device.imei = std::move(imeiParam);
  }
  inline void setDeviceIMSI(std::vector<char> imsiParam) noexcept {
    device.imsi = std::move(imsiParam);
  }
  inline void setDeviceLocation(std::vector<float> locationParam) noexcept {
    device.location = std::move(locationParam);
  }
  inline void setDeviceConfig(std::string configParam) noexcept {
    device.config = std::move(configParam);
  }
  inline void setDeviceNodes(std::string nodesParam) noexcept {
    device.nodes = std::move(nodesParam);
  }
  inline bool isWorking() const noexcept {
    return appWorkingState == WorkingState::WORKING;
  }
};

#endif  // APP_H
