#include "simtel.h"

#include <chrono>
#include <iostream>
#include <thread>

#include "EIRhandler.h"
#include "HLRhandler.h"
#include "VLRhandler.h"
#include "basestation.h"
#include "jsonparser.h"
#include "mme.h"
#include "registershandler.h"
#include "smsc.h"

SIMTEL::SIMTEL()
    : hlr(std::make_unique<HLR>()),
      vlr(std::make_unique<VLR>()),
      eir(std::make_unique<EIR>()),
      smsc(std::make_unique<SMSC>()),
      mme(std::make_unique<MME>()),
      is_running(false) {
  auto regHandler{
      std::make_unique<RegistersHandler>(hlr.get(), vlr.get(), eir.get())};
  mme->setRegistersHandler(std::move(regHandler));
  mme->setSMSC(smsc.get());
  smsc->setMME(mme.get());
  std::cout << "[SIMTEL] Initialized." << std::endl;
}

SIMTEL::~SIMTEL() { stop(); }

void SIMTEL::init(const std::string& config_path) {
  std::cout << "[SIMTEL] Loading config from: " << config_path << std::endl;
  JSONParser parser;
  try {
    parser.configurateSIMTEL(config_path, *this);
  } catch (const std::exception& e) {
    std::cerr << "[SIMTEL] Config error: " << e.what() << std::endl;
    throw;
  }
}

void SIMTEL::start() {
  bool expected{false};
  if (!is_running.compare_exchange_strong(expected, true)) {
    std::cerr << "[SIMTEL] Server is already started" << std::endl;
    return;
  }
  std::cout << "[SIMTEL] Starting BSs..." << std::endl;
  for (auto& bs : base_stations) {
    std::thread(&BS::work, bs.get()).detach();
  }
  std::cout << "[SIMTEL] Server started." << std::endl;
}

void SIMTEL::stop() {
  bool expected{true};
  if (!is_running.compare_exchange_strong(expected, false)) {
    return;
  }
  std::cout << "[SIMTEL] Stopping..." << std::endl;
  base_stations.clear();
}

void SIMTEL::interact() {
  if (!is_running) {
    std::cerr << "[ERROR] Server not started." << std::endl;
    return;
  }
  std::cout << "Type 'exit' for exiting." << std::endl;
  std::string line;
  while (is_running) {
    std::cout << "> ";
    std::getline(std::cin, line);
    if (line == "exit" || line == "quit") {
      stop();
      break;
    }
  }
}

void SIMTEL::addBaseStation(ENODE_B_ID id, const IPv4& ip, PORT port,
                            position_vector pos, BS_RADIUS radius) {
  auto bs{
      std::make_unique<BS>(id, mme.get(), ip, port, std::move(pos), radius)};

  mme->registerBaseStation(bs.get());

  base_stations.push_back(std::move(bs));
}