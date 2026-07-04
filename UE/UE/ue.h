#ifndef UE_H
#define UE_H

#include <iostream>
#include <thread>

#include "jsonparser.h"
#include "networkaddress.h"
#include "sock.h"
#include "common_types.h"
#include "socketBusinessWorker.h"

struct Device {
  IPv4 ip;
  IMEI imei;
  IMSI imsi;
  position_vector location;
  NetworkAddress serverAddress;
};

class UE {
 private:
  Device device;
  Status status{Status::NON_ACTIVE};
  WorkingState UEWorkingState{WorkingState::WORKING};
  SocketBusinessWorker socketBusinessWorker;
  TMSI tmsi_{0};
  std::thread receiverThread;
  std::atomic<bool> runningReceiver{false};
  std::mutex coutMutex;
  Sock socket_obj;
  std::atomic<bool> handover_requested{false};
  IPv4 handover_ip;
  PORT handover_port{0};
  IPv4 current_bs_ip;
  PORT current_bs_port;
  std::atomic<Status> targetStatus{Status::NON_ACTIVE}; 
  std::atomic<bool> statusChanged{false};

  void printMenu() const noexcept;
  COMMAND inputCommand();
  // ProcessACTIVE, ProcessMOVE, ProcessSMS, ProcessEXIT
  // get vector not by & because in processInteract() I use std::move()
  // for tokens as it's not used later there
  void ProcessACTIVE(TOKENS_VECTOR tokens);
  void ProcessMOVE(TOKENS_VECTOR tokens);
  void ProcessEXIT(TOKENS_VECTOR tokens);
  void ProcessSMS(TOKENS_VECTOR tokens);
  TOKENS_VECTOR interpretateInputCommand(std::string command_buffer);
  void turnStringIntoUpper(std::string& str) const;
  void handleIncomingMessage(BYTE command_type, const BYTE_VECTOR& msg); 


 public:
  ~UE();

  void work();
  bool attach();
  void detach();
  void startReceiver();
  void stopReceiver();
  void receiverLoop();
  void processInteract(); 
  void performHandover(const IPv4& ip, PORT port);

  inline void setTargetStatus(Status new_status) noexcept{ targetStatus = new_status; }
  inline void setStatusChanged(bool changed) noexcept{ statusChanged = changed; }
  inline Status getTargetStatus() const noexcept{return targetStatus.load();}
  size_t getMessageSize(BYTE command_type) const noexcept;

  inline void setTmsi(TMSI tmsi) noexcept {tmsi_ = tmsi;}
  inline FD getFd() const noexcept { return socket_obj.getFd();}
  inline Status getStatus() const noexcept { return status; }
  inline void setStatus(Status statusParam) noexcept { status = statusParam; }
  inline void setUEWorkingState(WorkingState UEWorkingStateParam) noexcept { UEWorkingState = UEWorkingStateParam;}
  inline TMSI getTmsi() const noexcept { return tmsi_; }
  // setters are using copy because when we execute them
  // we use std::move() in inputParam so it calls default move-constructor
  inline void setDeviceServerAddress(
      NetworkAddress serverAddressParam) noexcept {
    device.serverAddress = std::move(serverAddressParam);
  }
  inline void setDeviceIP(IPv4 ip) {device.ip = ip;}
  inline IPv4 getDeviceIP() const noexcept { return device.ip;}
  inline IPv4 getDeviceSocketIP() const noexcept {return device.serverAddress.getIpString();}
  inline PORT getDeviceSocketPort() const noexcept {return device.serverAddress.getPort();}
  inline void setDeviceIMEI(IMEI imeiParam) { device.imei = std::move(imeiParam);}
  inline IMEI getDeviceIMEI() const noexcept {return device.imei;}
  inline void setDeviceIMSI(IMSI imsiParam) { device.imsi = std::move(imsiParam);}
  inline void setDeviceLocation(position_vector locationParam) {device.location = std::move(locationParam);}
};

#endif  // UE_H
