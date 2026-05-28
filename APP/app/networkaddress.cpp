#include "networkaddress.h"

constexpr uint32_t THREE_BYTES{24};
constexpr uint32_t TWO_BYTES{16};
constexpr uint32_t ONE_BYTE{8};
constexpr uint32_t FULL_ONE_BYTE_MASK{0xFF};

NetworkAddress::NetworkAddress(uint64_t ip_address, uint16_t portParam)
    : port(portParam) { 
  uint32_t ip_address32_t{static_cast<uint32_t>(ip_address)};
  uint8_t byte0_LE{static_cast<uint8_t>((ip_address32_t >> THREE_BYTES) &
                                        FULL_ONE_BYTE_MASK)};
  uint8_t byte1_LE{
      static_cast<uint8_t>((ip_address32_t >> TWO_BYTES) & FULL_ONE_BYTE_MASK)};
  uint8_t byte2_LE{
      static_cast<uint8_t>((ip_address32_t >> ONE_BYTE) & FULL_ONE_BYTE_MASK)};
  uint8_t byte3_LE{static_cast<uint8_t>(ip_address32_t & FULL_ONE_BYTE_MASK)};
  ip = std::to_string(byte0_LE) + "." + std::to_string(byte1_LE) + "." +
       std::to_string(byte2_LE) + "." + std::to_string(byte3_LE);
}

NetworkAddress::NetworkAddress(
    const std::string& full_address) {  //"254.254.254.254:45111" format
  std::size_t colon_index{full_address.find(":")};
  ip = full_address.substr(0, colon_index);
  port = static_cast<uint16_t>(std::stoi(full_address.substr(colon_index + 1)));
}

NetworkAddress::NetworkAddress(const std::string& ip_adrressParam,
                               const std::string& portParam) {
  ip = ip_adrressParam;
  port = static_cast<uint16_t>(std::stoi(portParam));
}
