#include "networkaddress.h"

NetworkAddress::NetworkAddress(uint64_t ip_address_LE, uint16_t portParam) : port(portParam){       //LE = Little Endian
    uint32_t ip_address32_t{static_cast<uint32_t>(ip_address_LE)};
    uint8_t byte0_LE{static_cast<uint8_t>((ip_address32_t >> 24) & 0xFF)};
    uint8_t byte1_LE{static_cast<uint8_t>((ip_address32_t >> 16) & 0xFF)};
    uint8_t byte2_LE{static_cast<uint8_t>((ip_address32_t >> 8) & 0xFF)};
    uint8_t byte3_LE{static_cast<uint8_t>(ip_address32_t & 0xFF)};
    ip = std::to_string(byte0_LE) + "." + std::to_string(byte1_LE) + "." + std::to_string(byte2_LE) + "." + std::to_string(byte3_LE);
}

NetworkAddress::NetworkAddress(const std::string& full_address){     //"254.254.254.254:45111" format
    size_t colon_index{full_address.find(":")};
    ip = full_address.substr(0, colon_index);
    port = static_cast<uint16_t>(std::stoi(full_address.substr(colon_index+1)));
}

NetworkAddress::NetworkAddress(const std::string& ip_adrressParam, const std::string& portParam){
    ip = ip_adrressParam;
    port = static_cast<uint16_t>(std::stoi(portParam));
}
