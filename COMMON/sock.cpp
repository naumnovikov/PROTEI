#include "sock.h"

constexpr int DEFAULT_READ_VALUE{0};

Sock::Sock(int domain, int type, int protocol) {
  sock = socket(domain, type, protocol);
}

Sock::~Sock() { close(sock); }

bool Sock::recv_full(uint8_t* buf, size_t len) const noexcept {
  while (len > 0) {
    ssize_t read_bytes{recv(sock, buf, len, DEFAULT_READ_VALUE)};
    if (read_bytes <= 0) [[unlikely]] {
      return false;
    }
    buf += read_bytes;
    len -= read_bytes;
  }
  return true;
}

int Sock::bindListenerForConnections(const uint16_t port,
                                     const char* IP) const {
  struct sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(IP);

  return bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0;
}

int Sock::connectAppSocketToServer(uint16_t server_port,
                                   const char* server_IP) const noexcept {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  addr.sin_addr.s_addr = inet_addr(server_IP);
  if (addr.sin_addr.s_addr == INADDR_NONE) [[unlikely]] {
    return -1;
  }
  return connect(sock, (struct sockaddr*)&addr, sizeof(addr));
}