#include "active.h"

void Active::execute() {
  if (app.getStatus() == newStatus) {
    return;
  }
  app.setStatus(newStatus);
  SPDLOG_INFO("Status changed to {}",
              (newStatus == Status::ACTIVE ? "ACTIVE" : "NON_ACTIVE"));
  printStatus(newStatus);
  std::cout << "\n";
}
