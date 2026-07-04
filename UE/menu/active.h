#ifndef ACTIVE_H
#define ACTIVE_H

#include "menu.h"
#include "ue.h"

class Active : public Menu {
 private:
  UE& ue;
  Status newStatus;

 public:
  inline Active(UE& UEParam, Status newStatusParam)
      : ue(UEParam), newStatus(newStatusParam) {}
  void execute() override;
};

#endif  // ACTIVE_H
