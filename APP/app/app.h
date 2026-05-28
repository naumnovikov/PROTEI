#ifndef APP_H
#define APP_H

#include <iostream>
#include <utility>

#include "jsonparser.h"
#include "networkaddress.h"
#include "socketWorker.h"
#include "workingstate.h"

struct Device {
  NetworkAddress socket;
  std::vector<char> imei;
  std::vector<char> imsi;
  std::vector<float> location;
  std::string config;
  std::string nodes;
  std::string server_ip;
  uint16_t server_port;
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
  inline std::vector<float>& getDeviceLocation() noexcept { return device.location; }
  inline const Device& getDevice() const noexcept { return device; }

  //setters are using copy because when we execute them
  //we use std::move()
  //ex: app.setServerIP(std::move(server_ip));      [jsonparser.cpp]
  inline void setDeviceSocket(NetworkAddress socketParam) noexcept {
    device.socket = std::move(socketParam);
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
  inline void setServerIP(std::string server_ipParam) noexcept {
    device.server_ip = std::move(server_ipParam);
  }
  inline void setServerPORT(uint16_t server_portParam) noexcept {
    device.server_port = server_portParam;
  }
};

#endif  // APP_H
