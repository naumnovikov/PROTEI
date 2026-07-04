#include "ue.h"

#include <ctype.h>

#include <cmath>
#include <exception>
#include <numeric>
#include <sstream>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "active.h"
#include "exit.h"
#include "move.h"
#include "sms.h"

//TO DO:
// - devide handleIncomingMessage() to different case functions

void UE::handleIncomingMessage(BYTE command_type, const BYTE_VECTOR& msg) {
    switch (command_type) {
        case 'M': { 
          //Message MSG
            if (msg.size() < MESSAGE_MSG_HEADER_SIZE) { 
              SPDLOG_WARN("Malformed SMS"); 
              return; 
            }
            
            TMSI dst_tmsi_net;
            std::memcpy(&dst_tmsi_net, msg.data(), TMSI_SIZE);
            TMSI dst_tmsi{ntohl(dst_tmsi_net)};
            
            MSISDN src_msisdn{reinterpret_cast<const char*>(msg.data() + M_MSISDN_OFFSET), MSISDN_SIZE};
            src_msisdn = src_msisdn.c_str();
            
            uint32_t sms_id_net;
            std::memcpy(&sms_id_net, msg.data() + M_SMS_ID_OFFSET, SMS_ID_SIZE);
            uint32_t sms_id{ntohl(sms_id_net)};
            
            std::string text{reinterpret_cast<const char*>(msg.data() + MESSAGE_MSG_HEADER_SIZE), msg.size() - MESSAGE_MSG_HEADER_SIZE};
            text = text.c_str();
            
            {
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "\n[SMS from " << src_msisdn << "]: " << text << std::endl;
            }
            
            BYTE_VECTOR ack;
            ack.push_back('D');
            TMSI tmsi_net{htonl(tmsi_)};
            ack.insert(ack.end(), reinterpret_cast<BYTE*>(&tmsi_net), reinterpret_cast<BYTE*>(&tmsi_net) + TMSI_SIZE);
            uint32_t ack_sms_id_net{htonl(sms_id)};
            ack.insert(ack.end(), reinterpret_cast<BYTE*>(&ack_sms_id_net), reinterpret_cast<BYTE*>(&ack_sms_id_net) + SMS_ID_SIZE);
            ack.push_back(SUCCESS_BYTE);
            send(socket_obj.getFd(), ack.data(), ack.size(), TCP_VALUE);
            break;
        }
        case 'm': { 
          //Delivery Report (for server)
            if (msg.size() < m_MSG_SIZE) { 
              SPDLOG_WARN("Malformed delivery report"); return; 
            }
            TMSI tmsi_net;
            uint32_t sms_id_net;
            std::memcpy(&tmsi_net, msg.data(), TMSI_SIZE);
            std::memcpy(&sms_id_net, msg.data() + TMSI_SIZE, SMS_ID_SIZE);
            BYTE status{msg[8]};
            
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "\nSMS ID " << ntohl(sms_id_net) 
                      << (status ? " SUCCESSFULLY delivered" : " FAILED to deliver") << std::endl;
            break;
        }
        case 'T': {
          //TMSI Assignment
            if (msg.size() >= T_MSG_SIZE) {
                TMSI tmsi_net;
                std::memcpy(&tmsi_net, msg.data(), TMSI_SIZE);
                tmsi_ = ntohl(tmsi_net);
                setTmsi(tmsi_);
                std::lock_guard<std::mutex> lock(coutMutex);
                std::cout << "\n[TMSI assigned] " << tmsi_ << std::endl;
            }
            break;
        }
        case 'C': {
          // Coef reply
            if (msg.size() < COEF_REPLY_MSG_SIZE) {
              SPDLOG_WARN("Malformed Coef reply"); 
              return; 
            }
            CONNECTION_COEF coef{socketBusinessWorker.decodeFloatFromBEBytes(msg.data())};
            ENODE_B_ID bs_id{socketBusinessWorker.decodeIntFromBEBytes(msg.data() + COEF_SIZE)};
            
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "\n[Coef reply] Connection coef: " << coef << " (BS ID: " << bs_id << ")" << std::endl;
            break;
        }
        case 'H': {
          // handover
    if (msg.size() < H_MSG_SIZE) {
        SPDLOG_WARN("Malformed handover command");
        return;
    }
    uint32_t net_ip;
    std::memcpy(&net_ip, msg.data(), IP_SIZE);
    PORT net_port;
    std::memcpy(&net_port, msg.data() + IP_SIZE, PORT_SIZE);
    
    struct in_addr addr;
    addr.s_addr = net_ip;
    IPv4 ho_ip{inet_ntoa(addr)};
    PORT ho_port{ntohs(net_port)};

    {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cout << "\n[Handover] Switching to BS " << ho_ip << ":" << ho_port << "..." << std::endl;
    }

    std::thread([this, ho_ip, ho_port]() {
        this->detach(); 

        // Wait some time to let MME delete UE from VLR
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        this->current_bs_ip = ho_ip;
        this->current_bs_port = ho_port;
        this->tmsi_ = 0;

        if (this->socket_obj.initialize(AF_INET, SOCK_STREAM, TCP_VALUE) < 0) {
            SPDLOG_ERROR("Handover: Socket init failed");
            this->setTargetStatus(Status::NON_ACTIVE);
            return;
        }

        if (this->socket_obj.connectUESocketToBS(ho_port, ho_ip.c_str()) >= 0) {
            if (this->attach()) {
                this->setStatus(Status::ACTIVE);
                this->startReceiver(); 
                SPDLOG_INFO("Handover completed successfully");
            } else {
                this->socket_obj.close_socket();
                this->setTargetStatus(Status::NON_ACTIVE);
            }
        } else {
            this->socket_obj.close_socket();
            this->setTargetStatus(Status::NON_ACTIVE);
        }
    }).detach();

    break;
}
        case 'D': { 
          // Delivery acknowledgement (for UE)
            if (msg.size() < DELIVERY_REPLY_MSG_SIZE) {
                SPDLOG_WARN("Malformed delivery reply");
                return;
            }
            BYTE status{msg[0]};
            std::lock_guard<std::mutex> lock(coutMutex);
            std::cout << "\n[SMS status] " << (status ? "Delivered to recipient" : "Failed to deliver") << std::endl;
            break;
        }case 'S': {
          //SMS Queued
          if (msg.size() < S_MSG_SIZE){
            SPDLOG_WARN("Malformed delivery reply");
            return;
          }
          uint32_t sms_id_net{};
          std::memcpy(&sms_id_net, msg.data(), S_MSG_SIZE);
          std::lock_guard<std::mutex> lock(coutMutex);
          std::cout << "\n[SMS Status] Accepted by network (SMSC queue), SMS ID: " << ntohl(sms_id_net) << std::endl;
          break;
        }
        default:
            SPDLOG_WARN("Unknown message type: {}", command_type);
            break;
    }
}

void UE::performHandover(const IPv4& ip, PORT port) {
    if (socket_obj.getFd() != UNDEFINED_SOCKET) {
        close(socket_obj.getFd());
    }
    socket_obj = Sock(AF_INET, SOCK_STREAM, TCP_VALUE);
    current_bs_ip = ip;
    current_bs_port = port;
    tmsi_ = 0; 
}

void UE::startReceiver() {
  if (receiverThread.joinable()){
    return;
  }
  runningReceiver = true;
  receiverThread = std::thread(&UE::receiverLoop, this);
}

void UE::stopReceiver() {
    if (!runningReceiver && UEWorkingState == WorkingState::NOT_WORKING) {
        return;
    }

    SPDLOG_INFO("Shutting down receiver...");

    runningReceiver = false; 

    FD fd{socket_obj.getFd()};
    if (fd != UNDEFINED_SOCKET) {
        shutdown(fd, SHUT_RDWR); 
    }

    if (receiverThread.joinable()) {
        receiverThread.join();
        SPDLOG_INFO("Receiver thread stopped successfully.");
    }

    socket_obj.close_socket();

    UEWorkingState = WorkingState::NOT_WORKING;
    status = Status::NON_ACTIVE;
}

void UE::receiverLoop() {
    while (runningReceiver) {
        BYTE command_type;
        if (!socket_obj.recv_full(&command_type, ONE_BYTE)) {
            if (runningReceiver) {
                SPDLOG_INFO("Connection closed or recv error");

                detach(); 
                
                setTargetStatus(Status::NON_ACTIVE);
                setStatusChanged(true);
            }
            break; 
        }

        std::size_t payload_size{getMessageSize(command_type)}; 

        BYTE_VECTOR msg(payload_size);
        if (payload_size > 0) {
            if (!socket_obj.recv_full(msg.data(), payload_size)) {
                SPDLOG_ERROR("Failed to receive full payload for command {}", command_type);
                break;
            }
        }

        handleIncomingMessage(command_type, msg);
    }
}
size_t UE::getMessageSize(BYTE command_type) const noexcept{
    switch (command_type) {
        case 'M': return 163;
        case 'm': return 9;
        case 'A': return 4;
        case 'T': return 4;
        case 'C': return 8;
        case 'H': return 6;
        case 'D': return 1;
        case 'S': return 4; 
        case 'd': return 4;
        default: return 0;
    }
}

void UE::work() {
    std::cout << "Configurate device from JSON. Input JSON-filename: ";
    std::string json_filename;
    std::cin >> json_filename;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    JSONParser parser;
    try {
        parser.configurateUE(std::move(json_filename), *this);
        current_bs_ip = device.serverAddress.getIpString();
        current_bs_port = device.serverAddress.getPort();
    } catch (const std::exception& e) {
        SPDLOG_ERROR("Configuration error: {}", e.what());
        return;
    }

    while (UEWorkingState == WorkingState::WORKING) {
        if (statusChanged.load()) {
            if (targetStatus.load() == Status::ACTIVE && status != Status::ACTIVE) {
                if (socket_obj.initialize(AF_INET, SOCK_STREAM, TCP_VALUE) < 0) {
                    SPDLOG_ERROR("Socket initialization error");
                    targetStatus = Status::NON_ACTIVE;
                } else if (socket_obj.connectUESocketToBS(current_bs_port, current_bs_ip.c_str()) >= 0) {
                    if (attach()) {
                        status = Status::ACTIVE; 
                        startReceiver();
                        SPDLOG_INFO("UE connected and ACTIVE");
                    } else {
                        SPDLOG_ERROR("Attach rejected by network. Closing socket.");
                        socket_obj.close_socket(); 
                        targetStatus = Status::NON_ACTIVE;
                    }
                } else {
                    SPDLOG_ERROR("Connect failed during activation");
                    socket_obj.close_socket(); 
                    targetStatus = Status::NON_ACTIVE;
                }
            } else if (targetStatus.load() == Status::NON_ACTIVE && status == Status::ACTIVE) {
                detach();
                SPDLOG_INFO("UE disconnected and NON_ACTIVE");
            }
            statusChanged = false;
        }

        processInteract();

        if (handover_requested.load()) {
            SPDLOG_INFO("Handover requested. Switching...");
            detach(); 
            
            performHandover(handover_ip, handover_port);
            
            targetStatus = Status::ACTIVE;
            statusChanged = true;
            handover_requested = false;
        }

        // If EXIT input
        if (UEWorkingState != WorkingState::WORKING) {
            break; 
        }
    }
    SPDLOG_INFO("UE Stopped.");
}

void UE::printMenu() const noexcept {
  std::cout << "UE status: ";
  printStatus(status);
  std::cout << '\n';
  std::cout << "IMSI: " << device.imsi;
  std::cout << "\n1. ACTIVE\n";
  std::cout << "2. MOVE\n";
  std::cout << "3. SMS\n";
  std::cout << "4. EXIT\n";
}

TOKENS_VECTOR UE::interpretateInputCommand(
    std::string command_buffer) {
  TOKENS_VECTOR tokens;
  std::istringstream iss(std::move(command_buffer));
  std::string token;
  while (iss >> token) {
    if (!token.empty()) [[likely]] {
      tokens.push_back(std::move(token));
    }
  }
  return tokens;
}

void UE::turnStringIntoUpper(std::string& str) const {
  for (char& c : str) {
    c = std::toupper(c);
  }
}

void UE::ProcessACTIVE(TOKENS_VECTOR tokens) {
  if (tokens.size() < ACTIVE_COMMAND_MIN_TOKENS) [[unlikely]] {
    throw std::invalid_argument("ACTIVE without arguments");
  }

  std::string arg{tokens[1]};

  turnStringIntoUpper(arg);
  if (arg == "TRUE" || arg == "1") {
    Active cmd(*this, Status::ACTIVE);
    cmd.execute();
  } else if (arg == "FALSE" || arg == "0") {
    Active cmd(*this, Status::NON_ACTIVE);
    cmd.execute();
  } else [[unlikely]] {
    SPDLOG_WARN("Invalid argument for ACTIVE");
  }
}

void UE::ProcessMOVE(TOKENS_VECTOR tokens) {
  if (status == Status::NON_ACTIVE) {
    throw std::logic_error("Cannot move as status is NON_ACTIVE");
  }
  std::size_t tokens_quantity{tokens.size()};
  if (tokens_quantity < MOVE_COMMAND_MIN_TOKENS) [[unlikely]] {
    throw std::invalid_argument("MOVE must get at least 1 argument");
  } else if (tokens_quantity > MOVE_COMMAND_MAX_TOKENS) {
    SPDLOG_WARN("MOVE: too many arguments, skipping extras");
  }
  if (tokens_quantity > COORDINATES_QUANTITY) {
    tokens_quantity = MAX_MOVE_TOKENS_QUANTITY;
  }
  position_vector new_location = device.location;
  try {
    for (std::size_t i{1}; i < tokens_quantity; ++i) {
      new_location[i - 1] = std::stof(tokens[i]);
    }
    Move cmd(*this, new_location, socket_obj);
    cmd.execute();
  } catch (const std::invalid_argument& e) {
    SPDLOG_ERROR("MOVE: invalid number format: {}", e.what());
  } catch (const std::out_of_range& e) {
    SPDLOG_ERROR("MOVE: index out of range: {}", e.what());
  } catch (const std::bad_alloc& e) {
    SPDLOG_ERROR("MOVE: memory allocation failed: {}", e.what());
  }
  // If any other exception drops,
  // I catch it in processInteract()
  // in catch(...) so here's no double logging
}

void UE::ProcessEXIT(TOKENS_VECTOR tokens) {
    SPDLOG_INFO("Exit command received, shutting down");
    
    detach();
    
    setUEWorkingState(WorkingState::NOT_WORKING); 
}

UE::~UE() {
    detach(); // Деструктор тоже просто отключает сеть, если забыли
}

COMMAND UE::inputCommand() {
  std::cout << ">> ";
  std::string command_buffer;
  if (!std::getline(std::cin, command_buffer)) [[unlikely]] {
    UEWorkingState = WorkingState::NOT_WORKING;
    SPDLOG_ERROR("Input stream closed or error");
    throw std::runtime_error("Input stream closed or error");
  }
  return command_buffer;
}

void UE::ProcessSMS(TOKENS_VECTOR tokens) {
    if (status != Status::ACTIVE) {
        throw std::logic_error("Cannot send SMS as status is NON_ACTIVE");
    }

    if (tokens.size() < SMS_MIN_TOKENS_QUANTITY) [[unlikely]] {
        throw std::invalid_argument("Usage: SMS <MSISDN_dst> <text>");
    }

    MSISDN dst_msisdn{tokens[1]};
    std::string sms_text;
    
    for (size_t i{THIRD_SMS_TOKEN_INDEX}; i < tokens.size(); ++i) {
        if (i > THIRD_SMS_TOKEN_INDEX) {
            sms_text += ' ';
        }
        sms_text += tokens[i];
    }
    try{
      SMS cmd(*this, std::move(dst_msisdn), std::move(sms_text));
      cmd.execute(); 
      std::lock_guard<std::mutex> lock(coutMutex);
    }catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(coutMutex);
        std::cerr << "[Error] " << e.what() << std::endl;
    }
    
}

void UE::processInteract() {
  while (UEWorkingState == WorkingState::WORKING && !statusChanged && !handover_requested) {
    printMenu();
    std::string command_buffer;
    try {
      command_buffer = inputCommand();
    } catch (...) {
      break;
    }

    if (command_buffer.empty()) [[unlikely]] {
      continue;
    }
    std::vector<std::string> tokens{
        interpretateInputCommand(std::move(command_buffer))};

    if (tokens.empty()) {
      continue;
    }

    std::string input_command{tokens[0]};
    for (char& c : input_command) {
      c = std::toupper(c);
    }

    if (input_command == "ACTIVE") {
      try {
        ProcessACTIVE(std::move(tokens));
      } catch (const std::invalid_argument& e) {
        SPDLOG_ERROR("{}", e.what());
        continue;
      } catch (...) {
        SPDLOG_ERROR("Unknown ACTIVE error!");
        continue;
      }
    } else if (input_command == "MOVE") {
      try {
        ProcessMOVE(std::move(tokens));
      } catch (const std::logic_error& e) {
        SPDLOG_ERROR("{}", e.what());
        continue;
      } catch (...) {
        SPDLOG_ERROR("Unknown MOVE error!");
        continue;
      }
    } else if (input_command == "SMS") {
      try {
        ProcessSMS(std::move(tokens));
      } catch (const std::exception& e) {
        SPDLOG_ERROR("SMS error: {}", e.what());
        continue;
      } catch (...) {
        SPDLOG_ERROR("Unknown SMS error!");
        continue;
      }
    } else if (input_command == "EXIT") {
      try {
        ProcessEXIT(std::move(tokens));
      } catch (const std::logic_error& e) {
        SPDLOG_ERROR("{}", e.what());
        continue;
      } catch (...) {
        SPDLOG_ERROR("Unknown EXIT error!");
        continue;
      }
      break;
    } else [[unlikely]] {
      SPDLOG_ERROR("Unknown command: '{}'", input_command);
      continue;
    }
  }
}

// TO DO:
// - devide attach to different functions

bool UE::attach() {
    BYTE_VECTOR req(REQUEST_MSG_SIZE);
    req[0] = 'R';

    IMSI imsi_fixed{device.imsi};
    imsi_fixed.resize(MAX_IMSI_SIZE, PADDING_ZERO);
    std::memcpy(req.data() + ONE_BYTE, imsi_fixed.c_str(), MAX_IMSI_SIZE);

    BYTE* pos_ptr{req.data() + ONE_BYTE + MAX_IMSI_SIZE};
    for (size_t i{0}; i < COORDINATES_QUANTITY; ++i) {
        uint32_t bits;
        std::memcpy(&bits, &device.location[i], sizeof(float));
        bits = htonl(bits);
        std::memcpy(pos_ptr + i * FLOAT_SIZE, &bits, FLOAT_SIZE);
    }

    if (send(socket_obj.getFd(), req.data(), req.size(), TCP_VALUE) < 0) {
        SPDLOG_ERROR("attach: send 'R' failed");
        return false;
    }

    BYTE coef_buf[COEF_REPLY_MSG_SIZE];
    if (!socket_obj.recv_full(coef_buf, COEF_REPLY_MSG_SIZE)) {
        SPDLOG_ERROR("attach: no coefficient/ID response");
        return false;
    }

    uint32_t net_bits, net_enb_id;
    std::memcpy(&net_bits, coef_buf, UINT32_T_SIZE);
    std::memcpy(&net_enb_id, coef_buf + UINT32_T_SIZE, UINT32_T_SIZE);
    
    CONNECTION_COEF coef;
    std::memcpy(&coef, &net_bits, COEF_SIZE); 
    
    uint32_t received_enb_id{ntohl(net_enb_id)};

    if (received_enb_id == ERROR_ENODE_B_ID) {
        SPDLOG_ERROR("attach: Received invalid BS ID (0)");
        return false;
    }

    SPDLOG_INFO("attach: Signal OK, Connected to eNodeB ID: {}", received_enb_id);

    std::vector<uint8_t> attach_msg(ATTACH_MSG_SIZE, 0); 
    
    attach_msg[0] = 'A';
    
    std::memcpy(attach_msg.data() + ONE_BYTE, imsi_fixed.c_str(), MAX_IMSI_SIZE);
    
    IMEI imei_fixed{device.imei};
    imei_fixed.resize(IMEI_SIZE, '0');
    std::memcpy(attach_msg.data() + ONE_BYTE + MAX_IMSI_SIZE, imei_fixed.c_str(), IMEI_SIZE);
    
    uint32_t enb_id_be{htonl(received_enb_id)};
    std::memcpy(attach_msg.data() + ONE_BYTE + MAX_IMSI_SIZE + IMEI_SIZE, &enb_id_be, UINT32_T_SIZE);
    
    if (send(socket_obj.getFd(), attach_msg.data(), attach_msg.size(), TCP_VALUE) < 0) {
        SPDLOG_ERROR("attach: send 'A' failed");
        return false;
    }

    BYTE cmd_byte;
    if (!socket_obj.recv_full(&cmd_byte, ONE_BYTE)){
      SPDLOG_ERROR("attach: Network rejected attach request or connection closed.");
      return false;
    }
    
    if (cmd_byte != 'T' && cmd_byte != 'A') {
        SPDLOG_ERROR("attach: Expected 'T' or 'A', got '{}'", (char)cmd_byte);
        return false;
    }

    BYTE tmsi_buf[TMSI_SIZE];
    if (!socket_obj.recv_full(tmsi_buf, TMSI_SIZE)) {
        SPDLOG_ERROR("attach: no TMSI response");
        return false;
    }
    
    uint32_t net_tmsi;
    std::memcpy(&net_tmsi, tmsi_buf, TMSI_SIZE);
    tmsi_ = ntohl(net_tmsi);
    
    SPDLOG_INFO("attach: TMSI assigned = {}", tmsi_);
    return true;
}

void UE::detach() {
    if (status == Status::NON_ACTIVE) {
        return; 
    }

    SPDLOG_INFO("Detaching from network...");

    runningReceiver = false; 

    FD fd{socket_obj.getFd()};
    if (fd != UNDEFINED_SOCKET) {
        shutdown(fd, SHUT_RDWR); 
    }

    socket_obj.close_socket(); 

    if (receiverThread.joinable()) {
        if (std::this_thread::get_id() == receiverThread.get_id()) {
            receiverThread.detach();
        } else {
            receiverThread.join();
        }
    }

    status = Status::NON_ACTIVE;
    
    SPDLOG_INFO("UE disconnected from network. Ready for new attach.");
}

