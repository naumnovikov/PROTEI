#include "socketWorker.h"

#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/spdlog.h>

#include <string>
#include <vector>

#define THREE_BYTES 24
#define TWO_BYTES 16
#define ONE_BYTE 8
#define FULL_ONE_BYTE_MASK 0xFF
#define FLOAT_SIZE 4

constexpr float distanceError{-1.0f};

using json = nlohmann::json;

uint32_t SocketWorker::decodeUint32FromBEBytes(
    const uint8_t* data) const noexcept {
  return (uint32_t(data[0]) << THREE_BYTES) | (uint32_t(data[1]) << TWO_BYTES) |
         (uint32_t(data[2]) << ONE_BYTE) | uint32_t(data[3]);
}

float SocketWorker::decodeFloatFromBEBytes(const uint8_t* data) const noexcept {
  uint32_t bits{decodeUint32FromBEBytes(data)};
  float f;
  memcpy(&f, &bits, sizeof(f));
  return f;
}

bool SocketWorker::recv_full(int sock, uint8_t* buf,
                             std::size_t len) const noexcept {
  while (len > 0) {
    ssize_t read_bytes{recv(sock, buf, len, 0)};
    if (read_bytes <= 0) [[unlikely]] {
      return false;
    }
    buf += read_bytes;
    len -= read_bytes;
  }
  return true;
}

void SocketWorker::encodeFloatToBEBytes(float f, uint8_t* out) const noexcept {
  uint32_t bits;
  memcpy(&bits, &f, FLOAT_SIZE);
  out[0] = (bits >> THREE_BYTES) & FULL_ONE_BYTE_MASK;
  out[1] = (bits >> TWO_BYTES) & FULL_ONE_BYTE_MASK;
  out[2] = (bits >> ONE_BYTE) & FULL_ONE_BYTE_MASK;
  out[3] = bits & FULL_ONE_BYTE_MASK;
};

void SocketWorker::sendResponceToClient(int sock, uint8_t format,
                                        float distance, const char* client_ip,
                                        uint16_t client_port) const {
  std::vector<uint8_t> responce;
  if (format == binaryTypeByte) {
    constexpr uint32_t rest_len{protocol_len + binary_type_data_len};
    responce.reserve(size_len + rest_len);

    push4BytesInBE(responce, rest_len);

    responce.push_back(binaryTypeByte);

    uint8_t float_buf[float_len];
    encodeFloatToBEBytes(distance, float_buf);
    responce.insert(responce.end(), float_buf, float_buf + float_len);
  } else if (format == JSONTypeByte) {
    json worker;
    worker["distance"] = distance;

    std::string json_str{worker.dump()};
    uint32_t json_len{static_cast<uint32_t>(json_str.size())};
    uint32_t rest_len{protocol_len + json_len};
    responce.reserve(size_len + rest_len);

    push4BytesInBE(responce, rest_len);

    responce.push_back(JSONTypeByte);
    responce.insert(responce.end(), json_str.begin(), json_str.end());
  } else [[unlikely]] {
    SPDLOG_ERROR("[{}:{}] Unknown protocol format", client_ip, client_port);
    return;
  }
  send(sock, responce.data(), responce.size(), 0);
}

void SocketWorker::push4BytesInBE(std::vector<uint8_t>& msg,
                                  uint32_t data) const {
  msg.push_back((data >> 24) & 0xFF);
  msg.push_back((data >> 16) & 0xFF);
  msg.push_back((data >> 8) & 0xFF);
  msg.push_back(data & 0xFF);
}

void SocketWorker::fillMsgInBinaryFormatInBE(
    std::vector<uint8_t>& msg, const std::vector<float>& location) const {
  constexpr uint32_t data_len{static_cast<uint32_t>(binary_type_data_len)};
  constexpr uint32_t rest_len{static_cast<uint32_t>(protocol_len + data_len)};
  constexpr uint32_t total_len{static_cast<uint32_t>(size_len + rest_len)};

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

void SocketWorker::fillMsgInJSONFormatInBE(
    std::vector<uint8_t>& msg, const std::vector<float>& location) const {
  // msg consists of 3 parts in certain order:
  // [4 bytes for size of next 2 parts][1 byte for protocol type]...
  // ...[? bytes for coordinates in JSON]
  json worker;
  worker["location"] = {location[0], location[1], location[2]};
  std::string locationString{worker.dump()};
  uint32_t data_len{static_cast<uint32_t>(locationString.size())};
  uint32_t rest_len{protocol_len + data_len};
  uint32_t total_len{size_len + rest_len};

  msg.reserve(total_len);

  push4BytesInBE(msg, rest_len);

  msg.push_back(JSONTypeByte);

  msg.insert(msg.end(), locationString.begin(), locationString.end());
}

uint32_t SocketWorker::receiveRest_lenInLE(int sock) const {
  std::vector<uint8_t> rest_len_buf(4);

  if (!recv_full(sock, rest_len_buf.data(), size_len)) [[unlikely]] {
    SPDLOG_ERROR("recv rest_len_buf failed");
    throw std::runtime_error("recv rest_len_buf failed");
  }
  uint32_t tmp;
  memcpy(&tmp, rest_len_buf.data(), 4);
  uint32_t rest_len{ntohl(tmp)};

  if (rest_len == 0 || rest_len > 65536) [[unlikely]] {
    SPDLOG_ERROR("invalid rest_len {}", rest_len);
    throw std::out_of_range("invalid rest_len");
  }
  return rest_len;
}

float SocketWorker::getDistance(int sock, uint32_t rest_len) const {
  std::vector<uint8_t> rest(rest_len);

  if (!recv_full(sock, rest.data(), rest_len)) [[unlikely]] {
    SPDLOG_ERROR("recv rest failed");
    return distanceError;
  }

  if (rest_len < 1) [[unlikely]] {
    SPDLOG_ERROR("empty message");
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

int SocketWorker::bindListenerForConnections(const uint16_t port,
                                             const char* IP,
                                             int listenerForConnections) const {
  struct sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(IP);

  return bind(listenerForConnections, (struct sockaddr*)&addr, sizeof(addr)) <
         0;
}

bool SocketWorker::sendDistance(int client_sock, uint32_t rest_len,
                                bool& client_ok,
                                const std::vector<float>& position,
                                const char* client_ip,
                                uint16_t client_port) const {
  std::vector<uint8_t> rest(rest_len);

  if (!recv_full(client_sock, rest.data(), rest_len)) [[unlikely]] {
    client_ok = false;
    return false;
  }
  if (rest_len < 1) [[unlikely]] {
    SPDLOG_ERROR("[{}:{}] Empty message", client_ip, client_port);
    return false;
  }

  uint8_t format = rest[0];
  float x{0.0f}, y{0.0f}, z{0.0f};
  bool valid = false;

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
      json worker = json::parse(json_str);
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

  float distance =
      static_cast<float>(std::sqrt(std::pow(std::abs(position[0] - x), 2) +
                                   std::pow(std::abs(position[1] - y), 2) +
                                   std::pow(std::abs(position[2] - z), 2)));
  SPDLOG_INFO("[{}:{}] Received: ({}, {}, {}) -> distance = {}", client_ip,
               client_port, x, y, z, distance);

  sendResponceToClient(client_sock, format, distance, client_ip, client_port);
  return true;
}

int SocketWorker::connectAppSocketToServer(
    int sock, uint16_t server_port, const char* server_IP) const noexcept {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  addr.sin_addr.s_addr = inet_addr(server_IP);
  if (addr.sin_addr.s_addr == INADDR_NONE) [[unlikely]] {
    return -1;
  }
  return connect(sock, (struct sockaddr*)&addr, sizeof(addr));
}
