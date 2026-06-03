#ifndef SOCK_H
#define SOCK_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdint>

class Sock {
 private:
  int sock;

 public:
  Sock(int domain, int type, int protocol);
  inline explicit Sock(int existing_sock) { sock = existing_sock; };
  ~Sock();

  // restricting coping and allowing moving
  // to prevent a situation when
  // hold >1 version of the same socket
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

  inline int getSocket() const noexcept { return sock; }

  bool recv_full(uint8_t* buf, size_t len) const noexcept;
  int bindListenerForConnections(const uint16_t port, const char* IP) const;
  int connectAppSocketToServer(uint16_t server_port,
                               const char* server_IP) const noexcept;
};

#endif  // SOCK_H