#ifndef MOVE_H
#define MOVE_H

#include "menu.h"
#include "socketBusinessWorker.h"

class Move : public Menu {
 private:
  App& app;
  const std::vector<float>& newLocation;
  Sock& sock;
  SocketBusinessWorker socketBusinessWorker;

 public:
  Move(App& appParam, const std::vector<float>& newLocationParam,
       Sock& sockParam)
      : app(appParam), newLocation(newLocationParam), sock(sockParam) {}
  void execute() override;
};

#endif  // MOVE_H
