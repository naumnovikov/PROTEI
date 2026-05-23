#ifndef MOVE_H
#define MOVE_H

#include "menu.h"
#include "socketWorker.h"

class Move : public Menu {
 private:
  App& app;
  const std::vector<float>& newLocation;
  int sock;
  SocketWorker socketWorker;

 public:
  Move(App& appParam, const std::vector<float>& newLocationParam, int sockParam)
      : app(appParam), newLocation(newLocationParam), sock(sockParam) {}
  void execute() override;
};

#endif  // MOVE_H
