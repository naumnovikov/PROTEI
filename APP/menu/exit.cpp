#include "exit.h"

void Exit::execute() {
  app.setAppWorkingState(WorkingState::NOT_WORKING);
  SPDLOG_INFO("Exit command received, shutting down");
  std::cout << "Exiting...\n";
}
