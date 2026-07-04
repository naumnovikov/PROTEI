#ifndef MOVE_H
#define MOVE_H

#include "menu.h"
#include "socketBusinessWorker.h"
#include "ue.h"

class Move : public Menu {
 private:
  UE& ue;
  const position_vector& newLocation;
  Sock& sock;
  SocketBusinessWorker socketBusinessWorker;

 public:
  inline Move(UE& UEParam, const position_vector& newLocationParam,
       Sock& sockParam)
      : ue(UEParam), newLocation(newLocationParam), sock(sockParam) {}
  void execute() override;
};

#endif  // MOVE_H
