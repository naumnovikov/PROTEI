#ifndef EXIT_H
#define EXIT_H

#include "menu.h"

class Exit : public Menu {
 private:
  UE& ue;

 public:
  inline explicit Exit(UE& ueParam) : ue(ueParam) {}
  void execute() override;
};

#endif  // EXIT_H
