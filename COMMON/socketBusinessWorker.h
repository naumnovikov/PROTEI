#ifndef SOCKETBUSINESSWORKER_H
#define SOCKETBUSINESSWORKER_H

#include <exception>
#include <nlohmann/json.hpp>
#include <vector>

#include "sock.h"

constexpr uint32_t size_len{4};
constexpr uint32_t protocol_len{1};
constexpr uint32_t float_len{sizeof(float)};
constexpr uint32_t binary_type_data_len{3 * float_len};

constexpr uint8_t JSONTypeByte{0x00};
constexpr uint8_t binaryTypeByte{0x01};

constexpr int TCP_VALUE{0};

class SocketBusinessWorker {
 private:
  void encodeFloatToBEBytes(float f, uint8_t* out) const noexcept;
  void push4BytesInBE(std::vector<uint8_t>& msg, uint32_t data) const;
  void sendResponseToClient(int sock, uint8_t format, float distance,
                            const char* client_ip, uint16_t client_port) const;
  bool getCoordinates(uint32_t rest_len, const Sock& client_sock,
                      bool& client_ok, float& x, float& y, float& z,
                      const char* client_ip, uint16_t client_port,
                      uint8_t& format) const;

 public:
  float countDistance(float x_dif, float y_dif, float z_dif) const;
  float decodeFloatFromBEBytes(const uint8_t* data) const noexcept;
  bool sendDistance(const Sock& client_sock, uint32_t rest_len, bool& client_ok,
                    const std::vector<float>& position, const char* client_ip,
                    uint16_t client_port) const;
  void fillMsgInBinaryFormatInBE(std::vector<uint8_t>& msg,
                                 const std::vector<float>& location) const;
  void fillMsgInJSONFormatInBE(std::vector<uint8_t>& msg,
                               const std::vector<float>& location) const;
  uint32_t receiveRest_lenInLE(const Sock& sock) const;
  float getDistance(const Sock& sock, uint32_t rest_len) const;
  uint32_t decodeUint32FromBEBytes(const uint8_t* data) const noexcept;
};

#endif  // SOCKETBUSINESSWORKER_H