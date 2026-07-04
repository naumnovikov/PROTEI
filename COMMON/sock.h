#ifndef SOCK_H
#define SOCK_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "common_types.h"

class Sock {
 private:
  FD sock{-1};

 public:
  Sock() = default;
  Sock(int domain, int type, int protocol);
  inline explicit Sock(FD fd) { sock = fd; };
  ~Sock();

  // Restricting coping and allowing moving
  // to prevent a situation when
  // hold > 1 version of the same socket
  Sock(const Sock&) = delete;
  Sock& operator=(const Sock&) = delete;

  Sock(Sock&& other) noexcept : sock(other.sock) { other.sock = -1; }
  Sock& operator=(Sock&& other) noexcept {
    if (this != &other) {
      if (sock >= 0) {
        close(sock);
      }
      sock = other.sock;
      other.sock = -1;
    }
    return *this;
  }

  inline FD initialize(int domain, int type, int protocol) {
    sock = socket(domain, type, protocol);
    return sock;
  }
  inline FD initialize(FD fd) {
    sock = fd;
    return sock;
  }

  inline FD getFd() const noexcept { return sock; }
  inline void setFd(FD fd) { sock = fd; }

  bool recv_full(uint8_t* buf, size_t len) const noexcept;
  SUCCESS_RESULT bindListenerForConnections(const PORT port,
                                            const char* IP) const;
  SUCCESS_RESULT connectUESocketToBS(PORT server_port,
                                     const char* server_IP) const noexcept;
  void close_socket() noexcept;
};

#endif  // SOCK_H