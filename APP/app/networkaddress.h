#ifndef NETWORKADDRESS_H
#define NETWORKADDRESS_H

#include <cstdint>
#include <string>
#include <string_view>

using IPv4 = std::string;

#define LOCAL_IPv4_EXAMPLE "192.168.0.101"
#define LOCAL_PORT_EXAMPLE 2000

class NetworkAddress {
 private:
  IPv4 ip;
  uint16_t port;

 public:
  NetworkAddress() : ip(LOCAL_IPv4_EXAMPLE), port(LOCAL_PORT_EXAMPLE) {};
  NetworkAddress(const std::string& ip_adrressParam, uint16_t portParam)
      : ip(ip_adrressParam), port(portParam) {}
  NetworkAddress(uint64_t ip_address_LE, uint16_t portParam);
  explicit NetworkAddress(const std::string& full_address);
  NetworkAddress(const std::string& ip_adrressParam,
                 const std::string& portParam);

  std::string_view getIp() const { return ip; }
  std::string getIpString() const { return ip; }
  uint16_t getPort() const { return port; }
};

#endif  // NETWORKADDRESS_H
