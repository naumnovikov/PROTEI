#include "protocol.h"

void Protocol::execute() {
  if (app.getProtocol() == newProtocol) {
    SPDLOG_WARN("PROTOCOL: already set to requested protocol");
    return;
  }
  app.setProtocol(newProtocol);
  SPDLOG_INFO("Protocol changed to {}",
               (newProtocol == TypeOfProtocol::JSON ? "JSON" : "BINARY"));
  std::cout << "Protocol set to: ";
  printTypeOfProtocol(newProtocol);
  std::cout << "\n";
}
