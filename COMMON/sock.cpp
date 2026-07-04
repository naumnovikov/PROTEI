#include "sock.h"

Sock::Sock(int domain, int type, int protocol) {
  sock = socket(domain, type, protocol);
}

Sock::~Sock() { close_socket(); }

// As we use TCP which can devide msg on parts,
// we need to make sure we got all bytes
bool Sock::recv_full(uint8_t* buf, size_t len) const noexcept {
  while (len > 0) {
    ssize_t read_bytes{recv(sock, buf, len, TCP_VALUE)};
    if (read_bytes <= 0) [[unlikely]] {
      return false;
    }
    buf += read_bytes;
    len -= read_bytes;
  }
  return true;
}

SUCCESS_RESULT Sock::bindListenerForConnections(const PORT port,
                                                const char* IP) const {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(IP);

  return bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0;
}

SUCCESS_RESULT Sock::connectUESocketToBS(PORT BS_port,
                                         const char* BS_IP) const noexcept {
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(BS_port);
  addr.sin_addr.s_addr = inet_addr(BS_IP);
  if (addr.sin_addr.s_addr == INADDR_NONE) [[unlikely]] {
    return ERROR_CODE;
  }
  return connect(sock, (struct sockaddr*)&addr, sizeof(addr));
}

void Sock::close_socket() noexcept {
  if (sock != UNDEFINED_SOCKET) {
    close(sock);
    sock = UNDEFINED_SOCKET;
  }
}