#ifndef SOCKETWORKER_H
#define SOCKETWORKER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <nlohmann/json.hpp>

#include <exception>
#include <cstdint>
#include <cstring>

constexpr uint32_t size_len{4};   
constexpr uint32_t protocol_len{1}; 
constexpr uint32_t float_len{sizeof(float)};
constexpr uint32_t binary_type_data_len{3*float_len}; 
constexpr uint8_t JSONTypeByte{0x00}; 
constexpr uint8_t binaryTypeByte{0x01}; 

class SocketWorker{
private:
    void encodeFloatToBEBytes(float f, uint8_t* out) const noexcept;
    void push4BytesInBE(std::vector<uint8_t>& msg, uint32_t data) const;
    void sendResponceToClient(int sock, uint8_t format, float distance) const;
public:
    float decodeFloatFromBEBytes(const uint8_t* data) const noexcept;
    bool recv_full(int sock, uint8_t* buf, std::size_t len) const noexcept;
    int bindListenerForConnections(const uint16_t port, const char* IP, int listenerForConnections) const;
    bool sendDistance(int client_sock, float rest_len, bool& client_ok, const std::vector<float>& position) const;
    int connectAppSocketToServer(int sock, uint16_t server_port, const char* server_IP) const noexcept;
    void fillMsgInBinaryFormatInBE(std::vector<uint8_t>& msg, const std::vector<float>& location) const;
    void fillMsgInJSONFormatInBE(std::vector<uint8_t>& msg, const std::vector<float>& location) const;
    uint32_t receiveRest_lenInLE(int sock) const;
    float getDistance(int sock, uint32_t rest_len) const;
    uint32_t decodeUint32FromBEBytes(const uint8_t* data) const noexcept;
};

#endif // SOCKETWORKER_H