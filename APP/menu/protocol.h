#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "menu.h"

class Protocol : public Menu {
 private:
  App& app;
  TypeOfProtocol newProtocol;

 public:
  Protocol(App& appParam, TypeOfProtocol typeOfProtocolParam)
      : app(appParam), newProtocol(typeOfProtocolParam) {}
  void execute() override;
};

#endif  // PROTOCOL_H
