#ifndef SIMTEL_H
#define SIMTEL_H

#include <atomic>
#include <memory>

#include "common_types.h"

class HLR;
class VLR;
class EIR;
class SMSC;
class MME;
class BS;

class SIMTEL {
 private:
  std::unique_ptr<HLR> hlr;
  std::unique_ptr<VLR> vlr;
  std::unique_ptr<EIR> eir;
  std::unique_ptr<SMSC> smsc;
  std::unique_ptr<MME> mme;
  std::vector<std::unique_ptr<BS>> base_stations;
  std::atomic<bool> is_running;

 public:
  SIMTEL();
  ~SIMTEL();
  SIMTEL(const SIMTEL&) = delete;
  SIMTEL& operator=(const SIMTEL&) = delete;

  void init(const std::string& config_path);
  void start();
  void stop();
  void interact();
  void addBaseStation(ENODE_B_ID id, const IPv4& ip, PORT port,
                      position_vector pos, BS_RADIUS radius);
};

#endif