#ifndef EXIT_H
#define EXIT_H

#include "menu.h"

class Exit : public Menu {
 private:
  App& app;

 public:
  explicit Exit(App& appParam) : app(appParam) {}
  void execute() override;
};

#endif  // EXIT_H
