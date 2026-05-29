#ifndef NETWORKADDRESS_H
#define NETWORKADDRESS_H

#include <cstdint>
#include <string>
#include <string_view>

using IPv4 = std::string;

const std::string IPv4_EXAMPLE{"192.168.0.102"};
constexpr uint16_t PORT_EXAMPLE{2000};

class NetworkAddress {
 private:
  IPv4 ip;
  uint16_t port;

 public:
  NetworkAddress() : ip(IPv4_EXAMPLE), port(PORT_EXAMPLE) {};
  NetworkAddress(const std::string& ip_adrressParam, uint16_t portParam)
      : ip(ip_adrressParam), port(portParam) {}
  NetworkAddress(uint64_t ip_address, uint16_t portParam);
  explicit NetworkAddress(const std::string& full_address);
  NetworkAddress(const std::string& ip_adrressParam,
                 const std::string& portParam);

  inline std::string_view getIp() const { return ip; }
  inline std::string getIpString() const { return ip; }
  inline uint16_t getPort() const { return port; }
};

#endif  // NETWORKADDRESS_H
