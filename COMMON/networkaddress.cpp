#include "networkaddress.h"

NetworkAddress::NetworkAddress(uint64_t ip_address, PORT portParam)
    : port(portParam) {
  uint32_t ip_address32_t{static_cast<uint32_t>(ip_address)};
  BYTE byte0{
      static_cast<BYTE>((ip_address32_t >> THREE_BYTES) & FULL_ONE_BYTE_MASK)};
  BYTE byte1{
      static_cast<BYTE>((ip_address32_t >> TWO_BYTES) & FULL_ONE_BYTE_MASK)};
  BYTE byte2{static_cast<BYTE>((ip_address32_t >> ONE_BYTE_SHIFT) &
                               FULL_ONE_BYTE_MASK)};
  BYTE byte3{static_cast<BYTE>(ip_address32_t & FULL_ONE_BYTE_MASK)};
  ip = std::to_string(byte0) + "." + std::to_string(byte1) + "." +
       std::to_string(byte2) + "." + std::to_string(byte3);
}

NetworkAddress::NetworkAddress(
    const std::string& full_address) {  //"254.254.254.254:45111" format
  std::size_t colon_index{full_address.find(":")};
  ip = full_address.substr(0, colon_index);
  port = static_cast<uint16_t>(std::stoi(full_address.substr(colon_index + 1)));
}
