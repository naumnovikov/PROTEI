#ifndef APP_H
#define APP_H

#include <iostream>
#include <utility>

#include "jsonparser.h"
#include "networkaddress.h"
#include "sock.h"
#include "workingstate.h"

struct Device {
  NetworkAddress serverAddress;
  std::string imei;
  std::string imsi;
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

  void printMenu() const noexcept;
  std::string inputCommand();
  void processInteract(Sock& sock);

  // ProcessACTIVE, ProcessMOVE, ProcessEXIT, ProcessPROTOCOL
  // get vector not by & because in processInteract() I use std::move()
  // for tokens as it's not used later there
  void ProcessACTIVE(std::vector<std::string> tokens);
  void ProcessMOVE(std::vector<std::string> tokens, Sock& sock);
  void ProcessEXIT(std::vector<std::string> tokens);
  void ProcessPROTOCOL(std::vector<std::string> tokens);

  std::vector<std::string> interpretateInputCommand(std::string command_buffer);
  void configurate(std::string json_filenameParam);
  void turnStringIntoUpper(std::string& str) const;

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

  // setters are using copy because when we execute them
  // we use std::move() in inputParam so it calls default move-constructor
  inline void setDeviceServerAddress(
      NetworkAddress serverAddressParam) noexcept {
    device.serverAddress = std::move(serverAddressParam);
  }
  inline std::string getDeviceSocketIP() const noexcept {
    return device.serverAddress.getIpString();
  }
  inline uint16_t getDeviceSocketPort() const noexcept {
    return device.serverAddress.getPort();
  }
  inline void setDeviceIMEI(std::string imeiParam) noexcept {
    device.imei = std::move(imeiParam);
  }
   inline std::string getDeviceIMEI() noexcept {return device.imei;}
  inline void setDeviceIMSI(std::string imsiParam) noexcept {
    device.imsi = std::move(imsiParam);
  }
  inline std::string getDeviceIMSI() noexcept {return device.imsi;}
  inline std::vector<float> getDeviceLocation() const noexcept {
    return device.location;
  }
  inline void setDeviceLocation(std::vector<float> locationParam) noexcept {
    device.location = std::move(locationParam);
  }
  inline void setDeviceConfig(std::string configParam) noexcept {
    device.config = std::move(configParam);
  }
  inline std::string getDeviceConfig() noexcept {return device.config;}
  inline void setDeviceNodes(std::string nodesParam) noexcept {
    device.nodes = std::move(nodesParam);
  }
  inline std::string getDeviceNodes() noexcept {return device.nodes;}
  inline bool isWorking() const noexcept {
    return appWorkingState == WorkingState::WORKING;
  }
};

#endif  // APP_H
