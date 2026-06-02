#include "move.h"

void Move::execute() {
  std::vector<float> temp{app.getDeviceLocation()};
  for (std::size_t i{0}; i < newLocation.size(); ++i) {
    temp.at(i) = newLocation.at(i);
  }
  app.setDeviceLocation(temp);
  SPDLOG_INFO("Location changed to: {}, {}, {}", std::to_string(temp[0]),
              std::to_string(temp[1]), std::to_string(temp[2]));
  std::vector<uint8_t> msg;

  if (app.getProtocol() == TypeOfProtocol::BINARY) {
    socketBusinessWorker.fillMsgInBinaryFormatInBE(msg,
                                                   app.getDeviceLocation());
  } else {
    socketBusinessWorker.fillMsgInJSONFormatInBE(msg, app.getDeviceLocation());
  }

  if (send(sock.getSocket(), msg.data(), msg.size(), 0) < 0) [[unlikely]] {
    SPDLOG_ERROR("Send error");
    return;
  }

  uint32_t rest_len;
  try {
    rest_len = socketBusinessWorker.receiveRest_lenInLE(sock);
  } catch (const std::runtime_error& e) {
    return;
  } catch (const std::out_of_range& e) {
    return;
  } catch (...) {
    SPDLOG_ERROR("Unknown error with rest_len");
    return;
  }

  float distanceResult{socketBusinessWorker.getDistance(sock, rest_len)};

  if (distanceResult == -1.0f) [[unlikely]] {
    return;
  }

  std::cout << "Distance = " << distanceResult << '\n';
  SPDLOG_INFO("Received distance: {}", distanceResult);
}
