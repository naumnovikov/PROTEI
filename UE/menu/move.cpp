#include "move.h"

void Move::execute() {
  position_vector newCoords{newLocation};
  ue.setDeviceLocation(newCoords);
  BYTE_VECTOR msg;
  
  msg.push_back('U'); 

  BYTE buf[FLOAT_SIZE];
  
  auto addFloat {[&](float val) {
      BYTE b[FLOAT_SIZE];
      socketBusinessWorker.encodeFloatToBEBytes(val, b); 
      msg.insert(msg.end(), b, b + FLOAT_SIZE);
  }};

  addFloat(newCoords[0]);
  addFloat(newCoords[1]);
  addFloat(newCoords[2]);
  
  if (send(sock.getFd(), msg.data(), msg.size(), TCP_VALUE) < 0) {
    SPDLOG_ERROR("Send error");
  }
}