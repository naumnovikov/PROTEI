#ifndef NETWORKADDRESS_H
#define NETWORKADDRESS_H

#include <string>
#include <cstdint>
#include <string_view>

using IPv4 = std::string;

class NetworkAddress{
private:
    IPv4 ip;
    uint16_t port;
public:
    NetworkAddress() : ip("192.168.0.101"), port(2000) {};
    NetworkAddress(const std::string& ip_adrressParam, uint16_t portParam) : ip(ip_adrressParam), port(portParam){}
    NetworkAddress(uint64_t ip_address_LE, uint16_t portParam);
    explicit NetworkAddress(const std::string& full_address);
    NetworkAddress(const std::string& ip_adrressParam, const std::string& portParam);

    std::string_view getIp() const { return ip; }
    uint16_t getPort() const { return port; }
};

#endif // NETWORKADDRESS_H
