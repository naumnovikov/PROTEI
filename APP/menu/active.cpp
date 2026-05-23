#include "active.h"

void Active::execute() {
  if (app.getStatus() == newStatus) {
    SPDLOG_WARN("ACTIVE: status already set to the requested value");
    return;
  }
  app.setStatus(newStatus);
  SPDLOG_INFO("Status changed to {}",
               (newStatus == Status::ACTIVE ? "ACTIVE" : "NON_ACTIVE"));
  printStatus(newStatus);
  std::cout << "\n";
}
