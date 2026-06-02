#include "socketBusinessWorker.h"

#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/spdlog.h>

#include <cstring>
#include <string>

constexpr uint32_t THREE_BYTES{24};
constexpr uint32_t TWO_BYTES{16};
constexpr uint32_t ONE_BYTE{8};
constexpr uint32_t FULL_ONE_BYTE_MASK{0xFF};
constexpr uint32_t EMPTY_LEN{0};
constexpr uint32_t MAX_REST_LEN{65536};
constexpr float distanceError{-1.0f};
constexpr int FLOAT_SIZE{4};
constexpr int DEFAULT_SEND_VALUE{0};

using json = nlohmann::json;

uint32_t SocketBusinessWorker::decodeUint32FromBEBytes(
    const uint8_t* data) const noexcept {
  return (uint32_t(data[0]) << THREE_BYTES) | (uint32_t(data[1]) << TWO_BYTES) |
         (uint32_t(data[2]) << ONE_BYTE) | uint32_t(data[3]);
}

float SocketBusinessWorker::decodeFloatFromBEBytes(
    const uint8_t* data) const noexcept {
  uint32_t bits{decodeUint32FromBEBytes(data)};
  float f;
  memcpy(&f, &bits, sizeof(f));
  return f;
}

bool SocketBusinessWorker::getCoordinates(uint32_t rest_len,
                                          const Sock& client_sock,
                                          bool& client_ok, float& x, float& y,
                                          float& z, const char* client_ip,
                                          uint16_t client_port,
                                          uint8_t& format) const {
  if (rest_len < 1) [[unlikely]] {
    SPDLOG_ERROR("[{}:{}] Empty message", client_ip, client_port);
    client_ok = false;
    return false;
  }

  std::vector<uint8_t> rest(rest_len);

  if (!client_sock.recv_full(rest.data(), rest_len)) [[unlikely]] {
    client_ok = false;
    return false;
  }

  format = rest[0];
  bool valid{false};

  if (format == binaryTypeByte) {
    if (rest_len != protocol_len + binary_type_data_len) {
      SPDLOG_ERROR("[{}:{}] Binary: expected 12 bytes, got {}", client_ip,
                   client_port, rest_len - 1);
      return false;
    }
    x = decodeFloatFromBEBytes(rest.data() + protocol_len);
    y = decodeFloatFromBEBytes(rest.data() + protocol_len + float_len);
    z = decodeFloatFromBEBytes(rest.data() + protocol_len + float_len +
                               float_len);
    valid = true;
  } else if (format == JSONTypeByte) {
    std::string json_str(rest.begin() + 1, rest.end());
    try {
      json worker{json::parse(json_str)};
      if (worker.contains("location") && worker["location"].is_array() &&
          worker["location"].size() >= 3) {
        x = worker["location"][0].get<float>();
        y = worker["location"][1].get<float>();
        z = worker["location"][2].get<float>();
        valid = true;
      } else {
        SPDLOG_WARN("[{}:{}] JSON missing 'location' array", client_ip,
                    client_port);
      }
    } catch (const std::exception& e) {
      SPDLOG_WARN("[{}:{}] JSON parse error: {}", client_ip, client_port,
                  e.what());
    }
  } else [[unlikely]] {
    SPDLOG_WARN("[{}:{}] Unknown format", client_ip, client_port);
    return false;
  }

  if (!valid) [[unlikely]] {
    return false;
  }
  return true;
}

bool SocketBusinessWorker::sendDistance(const Sock& client_sock,
                                        uint32_t rest_len, bool& client_ok,
                                        const std::vector<float>& position,
                                        const char* client_ip,
                                        uint16_t client_port) const {
  float x{0.0f}, y{0.0f}, z{0.0f};
  uint8_t format;
  if (!getCoordinates(rest_len, client_sock, client_ok, x, y, z, client_ip,
                      client_port, format)) {
    return false;
  }

  float distance{
      countDistance(position[0] - x, position[1] - y, position[2] - z)};

  SPDLOG_INFO("[{}:{}] Received: ({}, {}, {}) -> distance = {}", client_ip,
              client_port, x, y, z, distance);

  sendResponseToClient(client_sock.getSocket(), format, distance, client_ip,
                       client_port);
  return true;
}

void SocketBusinessWorker::encodeFloatToBEBytes(float f,
                                                uint8_t* out) const noexcept {
  uint32_t bits;
  memcpy(&bits, &f, FLOAT_SIZE);
  out[0] = (bits >> THREE_BYTES) & FULL_ONE_BYTE_MASK;
  out[1] = (bits >> TWO_BYTES) & FULL_ONE_BYTE_MASK;
  out[2] = (bits >> ONE_BYTE) & FULL_ONE_BYTE_MASK;
  out[3] = bits & FULL_ONE_BYTE_MASK;
};

void SocketBusinessWorker::sendResponseToClient(int sock, uint8_t format,
                                                float distance,
                                                const char* client_ip,
                                                uint16_t client_port) const {
  std::vector<uint8_t> response;
  if (format == binaryTypeByte) {
    // msg consists of 3 parts in certain order:
    // [4 bytes for size of next 2 parts][1 byte for protocol type]...
    // ...[4 bytes for float]
    constexpr uint32_t rest_len{protocol_len + binary_type_data_len};
    response.reserve(size_len + rest_len);

    push4BytesInBE(response, rest_len);

    response.push_back(binaryTypeByte);

    uint8_t float_buf[float_len];
    encodeFloatToBEBytes(distance, float_buf);
    response.insert(response.end(), float_buf, float_buf + float_len);
  } else if (format == JSONTypeByte) {
    // msg consists of 3 parts in certain order:
    // [4 bytes for size of next 2 parts][1 byte for protocol type]...
    // ...[? bytes for JSON]
    json worker;
    worker["distance"] = distance;

    std::string json_str{worker.dump()};
    uint32_t json_len{static_cast<uint32_t>(json_str.size())};
    uint32_t rest_len{protocol_len + json_len};
    response.reserve(size_len + rest_len);

    push4BytesInBE(response, rest_len);

    response.push_back(JSONTypeByte);
    response.insert(response.end(), json_str.begin(), json_str.end());
  } else [[unlikely]] {
    SPDLOG_ERROR("[{}:{}] Unknown protocol format", client_ip, client_port);
    return;
  }
  send(sock, response.data(), response.size(), DEFAULT_SEND_VALUE);
}

void SocketBusinessWorker::push4BytesInBE(std::vector<uint8_t>& msg,
                                          uint32_t data) const {
  msg.push_back((data >> THREE_BYTES) & FULL_ONE_BYTE_MASK);
  msg.push_back((data >> TWO_BYTES) & FULL_ONE_BYTE_MASK);
  msg.push_back((data >> ONE_BYTE) & FULL_ONE_BYTE_MASK);
  msg.push_back(data & FULL_ONE_BYTE_MASK);
}

void SocketBusinessWorker::fillMsgInBinaryFormatInBE(
    std::vector<uint8_t>& msg, const std::vector<float>& location) const {
  constexpr uint32_t data_len{static_cast<uint32_t>(binary_type_data_len)};
  constexpr uint32_t rest_len{static_cast<uint32_t>(protocol_len + data_len)};
  constexpr uint32_t total_len{static_cast<uint32_t>(rest_len + rest_len)};

  msg.reserve(total_len);

  push4BytesInBE(msg, rest_len);
  msg.push_back(binaryTypeByte);

  uint8_t buf[4];
  encodeFloatToBEBytes(location[0], buf);
  msg.insert(msg.end(), buf, buf + float_len);
  encodeFloatToBEBytes(location[1], buf);
  msg.insert(msg.end(), buf, buf + float_len);
  encodeFloatToBEBytes(location[2], buf);
  msg.insert(msg.end(), buf, buf + float_len);
}

void SocketBusinessWorker::fillMsgInJSONFormatInBE(
    std::vector<uint8_t>& msg, const std::vector<float>& location) const {
  // msg consists of 3 parts in certain order:
  // [4 bytes for size of next 2 parts][1 byte for protocol type]...
  // ...[? bytes for coordinates in JSON]
  json worker;
  worker["location"] = {location[0], location[1], location[2]};
  std::string locationString{worker.dump()};
  uint32_t data_len{static_cast<uint32_t>(locationString.size())};
  uint32_t rest_len{protocol_len + data_len};
  uint32_t total_len{rest_len + rest_len};

  msg.reserve(total_len);

  push4BytesInBE(msg, rest_len);

  msg.push_back(JSONTypeByte);

  msg.insert(msg.end(), locationString.begin(), locationString.end());
}

uint32_t SocketBusinessWorker::receiveRest_lenInLE(const Sock& sock) const {
  std::vector<uint8_t> rest_len_buf(size_len);

  if (!sock.recv_full(rest_len_buf.data(), size_len)) [[unlikely]] {
    SPDLOG_ERROR("recv rest_len_buf failed");
    throw std::runtime_error("recv rest_len_buf failed");
  }
  uint32_t tmp;
  memcpy(&tmp, rest_len_buf.data(), size_len);
  uint32_t rest_len{ntohl(tmp)};

  if (rest_len == EMPTY_LEN || rest_len > MAX_REST_LEN) [[unlikely]] {
    SPDLOG_ERROR("invalid rest_len {}", rest_len);
    throw std::out_of_range("invalid rest_len");
  }
  return rest_len;
}

float SocketBusinessWorker::getDistance(const Sock& sock,
                                        uint32_t rest_len) const {
  if (rest_len < 1) [[unlikely]] {
    SPDLOG_ERROR("empty message");
    return distanceError;
  }

  std::vector<uint8_t> rest(rest_len);

  if (!sock.recv_full(rest.data(), rest_len)) [[unlikely]] {
    SPDLOG_ERROR("recv rest failed");
    return distanceError;
  }

  uint8_t format{rest[0]};
  float distanceResult;

  if (format == JSONTypeByte) {
    std::string json_str{rest.begin() + 1, rest.end()};
    try {
      json worker = json::parse(json_str);
      distanceResult = worker.value("distance", distanceError);
    } catch (const std::exception& e) {
      SPDLOG_WARN("JSON parse error: {}", e.what());
      return distanceError;
    }
  } else if (format == binaryTypeByte) {
    if (rest_len != (protocol_len + float_len)) {
      SPDLOG_ERROR("Binary: expected 5 bytes, got {}", rest_len);
      return distanceError;
    }
    distanceResult = decodeFloatFromBEBytes(rest.data() + 1);
  } else [[unlikely]] {
    SPDLOG_ERROR("Unknown format {}", format);
    return distanceError;
  }
  if (distanceResult == distanceError) [[unlikely]] {
    SPDLOG_ERROR("Error receiving distance");
    return distanceError;
  }
  return distanceResult;
}

float SocketBusinessWorker::countDistance(float x_dif, float y_dif,
                                          float z_dif) const {
  return static_cast<float>(std::sqrt(std::pow(std::abs(x_dif), 2) +
                                      std::pow(std::abs(y_dif), 2) +
                                      std::pow(std::abs(z_dif), 2)));
}