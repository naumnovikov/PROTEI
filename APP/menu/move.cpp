#include "move.h"

void Move::execute() {
  for (std::size_t i{0}; i < newLocation.size(); ++i) {
    app.getDeviceLocation().at(i) = newLocation.at(i);
  }
  SPDLOG_INFO("Location changed to: {}, {}, {}",
               std::to_string(app.getDeviceLocation()[0]),
               std::to_string(app.getDeviceLocation()[1]),
               std::to_string(app.getDeviceLocation()[2]));
  std::vector<uint8_t> msg;

  if (app.getProtocol() == TypeOfProtocol::BINARY) {
    socketWorker.fillMsgInBinaryFormatInBE(msg, app.getDeviceLocation());
  } else {
    socketWorker.fillMsgInJSONFormatInBE(msg, app.getDeviceLocation());
  }

  if (send(sock, msg.data(), msg.size(), 0) < 0) [[unlikely]] {
    SPDLOG_ERROR("Send error");
    return;
  }

  uint32_t rest_len;
  try {
    rest_len = socketWorker.receiveRest_lenInLE(sock);
  } catch (const std::runtime_error& e) {
    return;
  } catch (const std::out_of_range& e) {
    return;
  } catch (...) {
    SPDLOG_ERROR("Unknown error with rest_len");
    return;
  }

  float distanceResult{socketWorker.getDistance(sock, rest_len)};

  if (distanceResult == -1.0f) [[unlikely]] {
    return;
  }

  std::cout << "Distance = " << distanceResult << '\n';
  SPDLOG_INFO("Received distance: {}", distanceResult);
}
