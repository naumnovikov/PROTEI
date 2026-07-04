#include "basestation.h"
#include "ue.h"

#include <cmath>

#include <spdlog/spdlog.h>

CONNECTION_COEF BS::calculateConnectionCoef(position_vector UE_location){
    if (UE_location.size() < COORDINATES_QUANTITY || position.size() < COORDINATES_QUANTITY)[[unlikely]] {
        SPDLOG_ERROR("[eNode-B{}] Cannot calculate coef: invalid vector size! UE size: {}, BS size: {}", 
                     eNode_B_id, UE_location.size(), position.size());
        return -ERROR_CONNECTION_COEF;
    }

    float dx{UE_location[0] - position[0]};
    float dy{UE_location[1] - position[1]};
    float dz{UE_location[2] - position[2]};

    float distance{std::sqrt(dx * dx + dy * dy + dz * dz)};

    return 1.0f - (distance / serviceRadius);
}

//TO DO:
// - Split processClient() to different functions

void BS::processClient(std::shared_ptr<Sock> client_sock, ThreadPool& pool,
                                const char* client_ip, PORT client_port) {
    struct timeval tv;
    tv.tv_sec = UE_RESPONCE_TIMEOUT_SEC;
    tv.tv_usec = 0;
    if (setsockopt(client_sock->getFd(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) [[unlikely]]{
        SPDLOG_WARN("[{}:{}][eNode-B{}] Failed to set recv timeout", client_ip, client_port, eNode_B_id);
    }
    SPDLOG_INFO("[{}:{}][eNode-B{}] Client connected", client_ip, client_port, eNode_B_id);

    //ATTACH
    std::vector<uint8_t> msg_buf(REQUEST_MSG_SIZE);
    if (!client_sock->recv_full(msg_buf.data(), REQUEST_MSG_SIZE))[[unlikely]] {
        SPDLOG_ERROR("[{}:{}][eNode-B{}] Couldn't recv 'R' request", client_ip, client_port, eNode_B_id);
        return;
    }
    IMSI ue_imsi;
    position_vector ue_location;
    if (socketBusinessWorker.decodeRequestMsg(eNode_B_id, ue_imsi, ue_location, msg_buf, client_ip, client_port) < 0) {
        return;
    }
    
    CONNECTION_COEF connectionCoef{calculateConnectionCoef(ue_location)};
    socketBusinessWorker.sendCoefReply(connectionCoef, eNode_B_id, client_sock->getFd(), client_ip, client_port);
    if (connectionCoef < 0) {
        SPDLOG_INFO("[{}:{}][eNode-B{}] Connection coef < 0", client_ip, client_port, eNode_B_id);
        return;
    }

    msg_buf.clear();
    msg_buf.resize(ATTACH_MSG_SIZE);
    if (!client_sock->recv_full(msg_buf.data(), ATTACH_MSG_SIZE)) [[unlikely]]{
        SPDLOG_ERROR("[{}:{}][eNode-B{}] Couldn't recv 'A' request", client_ip, client_port, eNode_B_id);
        return;
    }

    ENODE_B_ID received_eNodeB_id;
    IMEI ue_imei;
    if (socketBusinessWorker.decodeAttachMsg(eNode_B_id, ue_imsi, ue_imei, received_eNodeB_id, msg_buf, client_ip, client_port) < 0) {
        return;
    }
    if (eNode_B_id != received_eNodeB_id) [[unlikely]]{
        SPDLOG_ERROR("[{}:{}][eNode-B{}] ID mismatch! Expected: {}, but received: {}", 
                     client_ip, client_port, eNode_B_id, eNode_B_id, received_eNodeB_id);
        return;
    }
    uint32_t UE_TMSI{mme->onAttachRequest(eNode_B_id, ue_imsi, ue_imei)};
    if (UE_TMSI == 0) {
        SPDLOG_ERROR("[{}:{}][eNode-B{}] MME rejected attach", client_ip, client_port, eNode_B_id);
        return;
    }

    {
        // Not to lock everything
        std::lock_guard<std::mutex> lock(containerMutex);
        ueContainer.emplace(UE_TMSI, UEContext(UE_TMSI, client_sock));
    }

    socketBusinessWorker.sendTMSIFromBSToUE(UE_TMSI, client_sock->getFd(), client_ip, client_port);

    bool is_client_connected{true};
    bool is_handover{false};

    try {
        while (eNode_B_state == WorkingState::WORKING) {
            BYTE msg_type;
            ssize_t number_read{recv(client_sock->getFd(), &msg_type, ONE_BYTE, MSG_PEEK)};
            if (number_read <= 0) {
                if (number_read == 0) {
                    SPDLOG_INFO("[{}:{}][eNode-B{}] Client closed connection", client_ip, client_port, eNode_B_id);
                } else  {
                    SPDLOG_ERROR("[{}:{}][eNode-B{}] recv error", client_ip, client_port, eNode_B_id);
                }
                break;
            }

            switch (msg_type) {
                case 'M': {
                    //Message MSG
                    BYTE_VECTOR buffer(SMS_FULL_SIZE);
                    if (!client_sock->recv_full(buffer.data(), SMS_FULL_SIZE)) {
                        SPDLOG_ERROR("[{}:{}][eNode-B{}] Failed to read SMS", client_ip, client_port, eNode_B_id);
                        continue;
                    }
                    TMSI src_tmsi;
                    MSISDN dst_msisdn;
                    SMS_ID sms_id;
                    std::string sms_text;
                    if (socketBusinessWorker.decodeMessageMsg(eNode_B_id, src_tmsi, dst_msisdn, sms_id, sms_text, buffer, client_ip, client_port) < 0) {
                        continue;
                    }

                    //Clearing padding terminating zeros
                    dst_msisdn = std::string(dst_msisdn.c_str());

                    DeliveryStatus status{mme->onSmsSend(src_tmsi, dst_msisdn, sms_text)};
                    if (status == DELIVERY_STATUS_ERROR) {
                        socketBusinessWorker.sendDeliveryReply(client_sock->getFd(), src_tmsi, sms_id, DELIVERY_STATUS_ERROR);
                    }
                    break;
                }
                case 'U': {
                    //location Update
                    std::vector<uint8_t> buffer(UPDATE_SIZE);
                    if (!client_sock->recv_full(buffer.data(), UPDATE_SIZE)) {
                        SPDLOG_ERROR("[{}:{}][eNode-B{}] Failed to read update", client_ip, client_port, eNode_B_id);
                        continue;
                    }
                    position_vector new_location(COORDINATES_QUANTITY);
                    for (int i = 0; i < COORDINATES_QUANTITY; ++i) {
                        // MSG_TYPE_SIZE used here because we need to skip MSG_TYPE
                        // in every iteration and get only new_location[i]
                        new_location[i] = socketBusinessWorker.decodeFloatFromBEBytes(buffer.data() + ONE_BYTE + i * FLOAT_SIZE);
                    }
                    SPDLOG_INFO("[{}:{}][eNode-B{}] Position updated to ({},{},{})", client_ip, client_port, eNode_B_id,
                                new_location[0], new_location[1], new_location[2]);

                
                    CONNECTION_COEF current_coef{calculateConnectionCoef(new_location)}; 
                    ENODE_B_ID target_bs_id{mme->getBestBSForLocation(new_location, eNode_B_id)};

                    if (target_bs_id != NO_OTHER_BS_CODE && target_bs_id != eNode_B_id) {
                        SPDLOG_INFO("[{}:{}][eNode-B{}] Handover required to BS {}", client_ip, client_port, eNode_B_id, target_bs_id);
                        auto target_bs{mme->getBSById(target_bs_id)};
                        socketBusinessWorker.sendHandoverCommand(client_sock->getFd(), target_bs->getIp(), target_bs->getPort(), client_ip, client_port);
                        mme->initiateHandover(UE_TMSI, target_bs_id);
                        
                        is_handover = true;
                        return;
                    }
                    SPDLOG_INFO("DEBUG: Handover condition failed (Target: {}, MyID: {})", target_bs_id, eNode_B_id);
                    break;
                    }
                case 'D': {
                    //Delivery acknowledgement
                    BYTE_VECTOR buffer(ACK_SIZE);
                    if (!client_sock->recv_full(buffer.data(), ACK_SIZE)) {
                        SPDLOG_ERROR("[{}:{}][eNode-B{}] Failed to read ack", client_ip, client_port, eNode_B_id);
                        continue;
                    }
                    TMSI tmsi;
                    SMS_ID sms_id;
                    BYTE status_byte;
                    // 
                    std::memcpy(&tmsi, buffer.data() + ONE_BYTE, UINT32_T_SIZE);
                    std::memcpy(&sms_id, buffer.data() + ONE_BYTE + UINT32_T_SIZE, INT_SIZE);
                    status_byte = buffer[STATUS_BYTE_START];
                    tmsi = ntohl(tmsi);
                    sms_id = ntohl(sms_id);
                    SPDLOG_INFO("[{}:{}][eNode-B{}] Delivery ack: TMSI={}, SMS_ID={}, status={}", client_ip, client_port, eNode_B_id, tmsi, sms_id, status_byte);
                    mme->onDeliveryReport(tmsi, sms_id, (status_byte == SUCCESS_BYTE));
                    break;
                }
                default:
                    char dummy;
                    // Skipping one byte until getting known command
                    recv(client_sock->getFd(), &dummy, ONE_BYTE, TCP_VALUE);
                    SPDLOG_WARN("[{}:{}][eNode-B{}] Unknown command type: {}", client_ip, client_port, eNode_B_id, msg_type);
                    break;
            }
        }
    } catch (const std::exception& e) {
        SPDLOG_ERROR("[{}:{}][eNode-B{}] Exception during client processing: {}", client_ip, client_port, eNode_B_id, e.what());
    }

    removeUE(UE_TMSI);

    if (!is_handover) {
        mme->onClientDisconnect(UE_TMSI);
    }
    SPDLOG_INFO("[{}:{}][eNode-B{}] Client disconnected", client_ip, client_port, eNode_B_id);
}

void BS::processClients(ThreadPool& pool) {
  while (eNode_B_state == WorkingState::WORKING) {
    fd_set readfds;
    struct timeval tv_for_connections;

    int listenerForConnections{listenerSocket.getFd()};

    FD_ZERO(&readfds);
    FD_SET(listenerForConnections, &readfds);

    tv_for_connections.tv_sec = CONNECTTION_TIMEOUT_SEC;
    tv_for_connections.tv_usec = 0;

    SUCCESS_RESULT activity{select(listenerForConnections + 1, &readfds, NULL, NULL, &tv_for_connections)};

    if (activity < 0) [[unlikely]]{
      SPDLOG_ERROR("[eNode-B{}] Listening socket error", eNode_B_id);
      return;
    } else if (activity == 0) {
      SPDLOG_WARN("[eNode-B{}] Zero connections to listening socket", eNode_B_id);
      return;
    }

    struct sockaddr_in client_addr;
    socklen_t addr_size{sizeof(client_addr)};

    FD client_sock_fd{accept(listenerForConnections, (struct sockaddr*)&client_addr, &addr_size)};
    if (client_sock_fd < 0) [[unlikely]] {
      SPDLOG_ERROR("[eNode-B{}] Accept error: {}", eNode_B_id, strerror(errno));
      continue;
    }

    auto client_socket{std::make_shared<Sock>(client_sock_fd)};

    IPv4 client_ip{inet_ntoa(client_addr.sin_addr)};
    PORT client_port{ntohs(client_addr.sin_port)};

    pool.queueJob([this, client_socket, &pool, client_ip, client_port]() {
      processClient(client_socket, pool, client_ip.c_str(), client_port);
    });
  }
}

void BS::work() {
  listenerSocket.initialize(AF_INET, SOCK_STREAM, TCP_VALUE);
  int listenerForConnections{listenerSocket.getFd()};
  if (listenerForConnections < 0) [[unlikely]] {
    SPDLOG_ERROR("[eNode-B{}]listenerForConnections-socket creation error", eNode_B_id);
    return;
  }

  int opt{1};
  if (setsockopt(listenerForConnections, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) [[unlikely]]{
    SPDLOG_WARN("[eNode-B{}] setsockopt(SO_REUSEADDR) failed: {}", eNode_B_id, strerror(errno));
  }

  if (listenerSocket.bindListenerForConnections(BSAddress.getPort(), BSAddress.getIpString().c_str()) < 0)
      [[unlikely]] {
    SPDLOG_ERROR("[eNode-B{}]Bind error", eNode_B_id);
    return;
  }

  if (listen(listenerForConnections, CONNECTIONS_QUEUE_SIZE_LIMIT) < 0)
      [[unlikely]] {
    SPDLOG_ERROR("[eNode-B{}]Listen error", eNode_B_id);
    return;
  }

  SPDLOG_INFO("[eNode-B{}]Base station listening on {}:{}", eNode_B_id, BSAddress.getIp(), BSAddress.getPort());

  ThreadPool pool;
  pool.start();

  try {
    processClients(pool);
  } catch (...) {
    SPDLOG_ERROR("[eNode-B{}]Unknown exception in processClients", eNode_B_id);
  }

  pool.stop();
  SPDLOG_INFO("[eNode-B{}]Base station stopped", eNode_B_id);
}

void BS::removeUE(TMSI tmsi) {
    std::lock_guard<std::mutex> lock(containerMutex);

    if (ueContainer.erase(tmsi) > 0) {
        SPDLOG_INFO("[eNode-B{}] UE with TMSI {} removed", eNode_B_id, tmsi);
    } else {
        SPDLOG_WARN("[eNode-B{}] Deletion try UE {} that isn't in container", eNode_B_id, tmsi);
    }
}

std::shared_ptr<Sock> BS::getSocketByTMSI(TMSI tmsi) {
    std::lock_guard<std::mutex> lock(containerMutex);
    auto it{ueContainer.find(tmsi)};
    if (it != ueContainer.end()) {
        return it->second.socket; 
    }
    return nullptr;
}

bool BS::deliverSms(TMSI tmsiDst, std::string_view msisdnSrc, SMS_ID smsId, std::string_view SMS_Text) {
    auto client_sock{getSocketByTMSI(tmsiDst)};
    
    if (!client_sock) {
        SPDLOG_WARN("[eNode-B{}] deliverSms: UE with TMSI {} not found", eNode_B_id, tmsiDst);
        return false;
    }

    FD sock_fd{client_sock->getFd()};
    
    BYTE_VECTOR msg{socketBusinessWorker.encodeDeliveryMsg(tmsiDst, msisdnSrc, smsId, SMS_Text)};
    if (send(sock_fd, msg.data(), msg.size(), TCP_VALUE) < 0) {
        SPDLOG_ERROR("[eNode-B{}] deliverSms send failed: {}", eNode_B_id, strerror(errno));
        return false;
    } 
    
    SPDLOG_INFO("[eNode-B{}] deliverSms sent to TMSI {}", eNode_B_id, tmsiDst);
    return true;
}

void BS::sendDeliveryReport(TMSI tmsi, SMS_ID smsId, bool success) {
    auto client_sock{getSocketByTMSI(tmsi)};
    if (!client_sock){
        return;
    }
    
    socketBusinessWorker.sendDeliveryReply(client_sock->getFd(), tmsi, smsId, success ? 1 : 0);
}