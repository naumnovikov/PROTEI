#include "exit.h"

void Exit::execute() {
  app.setAppWorkingState(WorkingState::NOT_WORKING);
  spdlog::info("Exit command received, shutting down");
  std::cout << "Exiting...\n";
}
