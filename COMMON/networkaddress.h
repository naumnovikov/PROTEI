#ifndef NETWORKADDRESS_H
#define NETWORKADDRESS_H

#include "common_types.h"

class NetworkAddress {
 private:
  IPv4 ip;
  uint16_t port;

 public:
  inline NetworkAddress() : ip(IPv4_EXAMPLE), port(PORT_EXAMPLE) {};
  inline NetworkAddress(const IPv4& ip_adrressParam, uint16_t portParam)
      : ip(ip_adrressParam), port(portParam) {}
  NetworkAddress(uint64_t ip_address, PORT portParam);
  explicit NetworkAddress(const std::string& full_address);

  // IMPORTANT:
  // IF WE USE THIS CONSTRUCTOR, WE NEED TO
  // DO IT INSIDE TRY...CATCH
  // BECAUSE OF std::stoi
  inline NetworkAddress(const IPv4& ip_adrressParam,
                        const std::string& portParam) {
    ip = ip_adrressParam;
    port = static_cast<PORT>(std::stoi(portParam));
  }

  inline void initialize(const IPv4& ip_adrressParam, PORT portParam) {
    ip = ip_adrressParam;
    port = portParam;
  }

  inline std::string_view getIp() const { return ip; }
  inline std::string getIpString() const { return ip; }
  inline PORT getPort() const { return port; }
};

#endif  // NETWORKADDRESS_H
