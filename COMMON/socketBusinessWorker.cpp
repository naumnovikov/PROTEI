#include "socketBusinessWorker.h"

#include <spdlog/fmt/bundled/ranges.h>
#include <spdlog/spdlog.h>

#include <cstring>

uint32_t SocketBusinessWorker::decodeUint32FromBEBytes(
    const BYTE* data) const noexcept {
  return (uint32_t(data[0]) << THREE_BYTES) | (uint32_t(data[1]) << TWO_BYTES) |
         (uint32_t(data[2]) << ONE_BYTE_SHIFT) | uint32_t(data[3]);
}

float SocketBusinessWorker::decodeFloatFromBEBytes(
    const BYTE* data) const noexcept {
  uint32_t bits{decodeUint32FromBEBytes(data)};
  float f;
  std::memcpy(&f, &bits, sizeof(f));
  return f;
}

int SocketBusinessWorker::decodeIntFromBEBytes(
    const BYTE* data) const noexcept {
  uint32_t bits{decodeUint32FromBEBytes(data)};
  int f;
  std::memcpy(&f, &bits, sizeof(f));
  return f;
}

void SocketBusinessWorker::encodeFloatToBEBytes(float f,
                                                BYTE* out) const noexcept {
  uint32_t bits;
  std::memcpy(&bits, &f, FLOAT_SIZE);
  out[0] = (bits >> THREE_BYTES) & FULL_ONE_BYTE_MASK;
  out[1] = (bits >> TWO_BYTES) & FULL_ONE_BYTE_MASK;
  out[2] = (bits >> ONE_BYTE_SHIFT) & FULL_ONE_BYTE_MASK;
  out[3] = bits & FULL_ONE_BYTE_MASK;
};

void SocketBusinessWorker::push4BytesInBE(BYTE_VECTOR& msg,
                                          uint32_t data) const {
  msg.push_back((data >> THREE_BYTES) & FULL_ONE_BYTE_MASK);
  msg.push_back((data >> TWO_BYTES) & FULL_ONE_BYTE_MASK);
  msg.push_back((data >> ONE_BYTE_SHIFT) & FULL_ONE_BYTE_MASK);
  msg.push_back(data & FULL_ONE_BYTE_MASK);
}

SUCCESS_RESULT SocketBusinessWorker::decodeRequestMsg(
    ENODE_B_ID eNode_b_receiver_id, IMSI& ue_imsi, position_vector& ue_location,
    BYTE_VECTOR& msg_buf, const char* client_ip, PORT client_port) const {
  // Request msg consists of 3 parts:
  // 1) 1 byte for char 'R',
  // 2) 15 bytes for string IMSI,
  // 3) 12 bytes for UEs' position coordinates.

  if (msg_buf[MSG_TYPE_INDEX] != 'R') {
    SPDLOG_ERROR("[{}:{}][eNode-B{}] First byte is not 'R' in Request msg",
                 client_ip, client_port, eNode_b_receiver_id);
    return ERROR_CODE;
  }

  size_t len{IMSI_REQUEST_END - IMSI_REQUEST_START};
  ue_imsi.resize(len);
  std::memcpy(&ue_imsi[0], &msg_buf[IMSI_REQUEST_START], len);

  ue_location.clear();
  ue_location.reserve(COORDINATES_QUANTITY);
  for (size_t i{0}; i < COORDINATES_QUANTITY; ++i) {
    ue_location.push_back(decodeFloatFromBEBytes(
        msg_buf.data() + COORDINATES_REQUEST_START + FLOAT_SIZE * i));
  }
  return OK_CODE;
}

SUCCESS_RESULT SocketBusinessWorker::sendCoefReply(CONNECTION_COEF coef,
                                                   ENODE_B_ID eNodeB_id,
                                                   FD client_fd,
                                                   const char* client_ip,
                                                   PORT client_port) const {
  // Coef reply msg consists of 2 parts:
  // 1) 4 bytes for float coefficient,
  // 2) 4 bytes for eNodeB ID.

  BYTE_VECTOR msg(COEF_REPLY_MSG_SIZE);

  uint32_t coef_bytes;
  std::memcpy(&coef_bytes, &coef, COEF_BYTES_QUANTITY_IN_COEF_REPLY_MSG);
  uint32_t coef_be{htonl(coef_bytes)};
  std::memcpy(msg.data(), &coef_be, COEF_BYTES_QUANTITY_IN_COEF_REPLY_MSG);

  uint32_t id_be{htonl(eNodeB_id)};
  std::memcpy(msg.data() + ENODE_B_ID_BYTES_QUANTITY_IN_COEF_REPLY_MSG, &id_be,
              ENODE_B_ID_BYTES_QUANTITY_IN_COEF_REPLY_MSG);

  if (send(client_fd, msg.data(), COEF_REPLY_MSG_SIZE, TCP_VALUE) < 0) {
    SPDLOG_ERROR("Failed to send Coef reply");
    return ERROR_CODE;
  }
  return OK_CODE;
}

SUCCESS_RESULT SocketBusinessWorker::sendTMSIFromBSToUE(
    TMSI TMSI, FD sock, const char* client_ip, PORT client_port) const {
  // TMSI reply msg consists of 2 parts:
  // 1) 1 byte for char 'T',
  // 2) 4 bytes for TMSI.

  BYTE_VECTOR msg;
  msg.reserve(MSG_TYPE_BYTES_QUANTITY +
              TMSI_BYTES_QUANTITY_IN_TMSI_FROM_BS_TO_UE_MSG);
  msg.push_back('T');
  push4BytesInBE(msg, TMSI);
  if (send(sock, msg.data(), msg.size(), TCP_VALUE) < 0) {
    SPDLOG_ERROR("Failed to send TMSI from BS to UE");
    return ERROR_CODE;
  }
  return OK_CODE;
}

SUCCESS_RESULT SocketBusinessWorker::decodeAttachMsg(
    ENODE_B_ID eNode_b_receiver_id, IMSI& ue_imsi, IMEI& ue_imei,
    ENODE_B_ID& eNode_B_id, BYTE_VECTOR& msg_buf, const char* client_ip,
    PORT client_port) const {
  // Attach msg consists of 5 parts:
  // 1) 1 byte for char 'A',
  // 2) 15 bytes for string IMSI,
  // 3) 15 bytes for string IMEI,
  // 4) 4 bytes for eNode_B_id.

  if (msg_buf[MSG_TYPE_INDEX] != 'A') {
    SPDLOG_ERROR("[{}:{}][eNode-B{}] First byte is not 'A' in Request msg",
                 client_ip, client_port, eNode_B_id);
    return ERROR_CODE;
  }

  size_t len{IMSI_ATTACH_END - IMSI_ATTACH_START};
  ue_imsi.resize(len);
  std::memcpy(&ue_imsi[0], &msg_buf[IMSI_ATTACH_START], len);

  len = IMEI_ATTACH_END - IMEI_ATTACH_START;
  ue_imei.resize(len);
  std::memcpy(&ue_imei[0], &msg_buf[IMEI_ATTACH_START], len);

  eNode_B_id = decodeIntFromBEBytes(msg_buf.data() + ENODEB_ID_ATTACH_START);

  return OK_CODE;
}

SUCCESS_RESULT SocketBusinessWorker::decodeMessageMsg(
    ENODE_B_ID eNode_b_receiver_id, TMSI& TMSI_src, MSISDN& MSISDN_dst,
    SMS_ID& smsId, std::string& SMS_Text, BYTE_VECTOR& msg_buf,
    const char* client_ip, PORT client_port) const {
  // Message msg consists of 5 parts:
  // 1) 1 byte for char 'M',
  // 2) 4 bytes for string TMSI,
  // 3) 15 bytes for string MSISDN,
  // ("The ITU-T recommendation E.164 limits the maximum length of an MSISDN to
  // 15 digits." on https://en.wikipedia.org/wiki/MSISDN), 4) 140 bytes for SMS,
  // "The text messages to be transferred by means of the SM MT or SM MO contain
  // up to 140 octets." on https://clck.ru/3U3sgV.

  if (msg_buf[MSG_TYPE_INDEX] != 'M') {
    SPDLOG_ERROR("[{}:{}][eNode-B{}] First byte is not 'M' in Request msg",
                 client_ip, client_port, eNode_b_receiver_id);
    return ERROR_CODE;
  }

  smsId = decodeUint32FromBEBytes(msg_buf.data() + SMSID_MESSAGE_START);

  TMSI_src = decodeUint32FromBEBytes(msg_buf.data() + TMSI_MESSAGE_START);

  size_t len{MSISDN_MESSAGE_END - MSISDN_MESSAGE_START};
  MSISDN_dst.resize(len);
  std::memcpy(&MSISDN_dst[0], &msg_buf[MSISDN_MESSAGE_START], len);

  len = SMS_MESSAGE_END - SMS_MESSAGE_START;
  SMS_Text.resize(len);
  std::memcpy(&SMS_Text[0], &msg_buf[SMS_MESSAGE_START], len);

  return OK_CODE;
}

SUCCESS_RESULT SocketBusinessWorker::sendDeliveryReply(
    FD sock, TMSI tmsi, SMS_ID sms_id, DeliveryStatus status) const {
  // Delivery reply msg consists of 4 parts:
  // 1) 1 byte for char 'm',
  // 2) 4 bytes for TMSI,
  // 3) 4 bytes for SMS ID,
  // 4) 1 byte for DeliveryStatus.

  BYTE_VECTOR msg;
  msg.reserve(MSG_TYPE_BYTES_QUANTITY +
              TMSI_BYTES_QUANTITY_IN_DELIVERY_REPLY_MSG +
              SMS_ID_BYTES_QUANTITY_IN_DELIVERY_REPLY_MSG +
              DELIVERY_STATUS_BYTES_QUANTITY_IN_DELIVERY_REPLY_MSG);
  msg.push_back('m');

  uint32_t net_tmsi{htonl(tmsi)};
  msg.insert(msg.end(), reinterpret_cast<BYTE*>(&net_tmsi),
             reinterpret_cast<BYTE*>(&net_tmsi) +
                 TMSI_BYTES_QUANTITY_IN_DELIVERY_REPLY_MSG);

  uint32_t net_sms_id{htonl(sms_id)};
  msg.insert(msg.end(), reinterpret_cast<BYTE*>(&net_sms_id),
             reinterpret_cast<BYTE*>(&net_sms_id) +
                 SMS_ID_BYTES_QUANTITY_IN_DELIVERY_REPLY_MSG);

  msg.push_back(static_cast<BYTE>(status));

  if (send(sock, msg.data(), msg.size(), TCP_VALUE) < 0) {
    return ERROR_CODE;
  }
  return OK_CODE;
}

SUCCESS_RESULT SocketBusinessWorker::sendHandoverCommand(
    FD sock, const IPv4& bs_ip, PORT bs_port, const char* client_ip,
    PORT client_port) const {
  // Handover command msg consists of 3 parts:
  // 1) 1 byte for char 'H',
  // 2) 4 bytes for IPv4 address,
  // 3) 2 bytes for port.

  BYTE_VECTOR msg;
  msg.reserve(MSG_TYPE_BYTES_QUANTITY +
              IPv4_BYTES_QUANTITY_IN_HANDOVER_COMMAND_MSG +
              PORT_BYTES_QUANTITY_IN_HANDOVER_COMMAND_MSG);
  msg.push_back('H');

  uint32_t net_ip{inet_addr(bs_ip.c_str())};
  msg.insert(msg.end(), reinterpret_cast<BYTE*>(&net_ip),
             reinterpret_cast<BYTE*>(&net_ip) +
                 IPv4_BYTES_QUANTITY_IN_HANDOVER_COMMAND_MSG);

  uint16_t net_port = htons(bs_port);
  msg.insert(msg.end(), reinterpret_cast<BYTE*>(&net_port),
             reinterpret_cast<BYTE*>(&net_port) +
                 PORT_BYTES_QUANTITY_IN_HANDOVER_COMMAND_MSG);

  if (send(sock, msg.data(), msg.size(), TCP_VALUE) < 0) {
    SPDLOG_ERROR("[{}:{}] sendHandoverCommand failed.", client_ip, client_port);
    return ERROR_CODE;
  }
  return OK_CODE;
}

BYTE_VECTOR SocketBusinessWorker::encodeDeliveryMsg(
    TMSI tmsiDst, std::string_view msisdnSrc, SMS_ID smsId,
    std::string_view SMS_Text) const {
  // Delivery message consists of 5 parts:
  // 1) 1 byte for char 'M',
  // 2) 4 bytes for TMSI,
  // 3) 15 bytes for MSISDN (padded with zeros),
  // 4) 4 bytes for SMS ID,
  // 5) 140 bytes for SMS Text (padded with zeros).

  BYTE_VECTOR msg;
  msg.reserve(MSG_TYPE_BYTES_QUANTITY + TMSI_BYTES_QUANTITY_IN_DELIVERY_MSG +
              MSISDN_BYTES_QUANTITY_IN_DELIVERY_MSG +
              SMS_ID_BYTES_QUANTITY_IN_DELIVERY_MSG +
              SMS_TEXT_BYTES_QUANTITY_IN_DELIVERY_MSG);

  msg.push_back('M');

  uint32_t net_tmsi{htonl(tmsiDst)};

  // Doesn`t result in Strict Aliasing Rule violation
  // because of using BYTE* which is actually uint8_t*
  auto* tmsi_ptr{reinterpret_cast<BYTE*>(&net_tmsi)};
  msg.insert(msg.end(), tmsi_ptr, tmsi_ptr + sizeof(net_tmsi));

  BYTE msisdn_buf[MSISDN_SIZE]{0};
  size_t msisdn_len{
      std::min(msisdnSrc.size(), static_cast<size_t>(MSISDN_SIZE))};
  std::memcpy(msisdn_buf, msisdnSrc.data(), msisdn_len);
  msg.insert(msg.end(), msisdn_buf, msisdn_buf + MSISDN_SIZE);

  uint32_t net_sms_id{htonl(static_cast<uint32_t>(smsId))};
  auto* sms_id_ptr{reinterpret_cast<BYTE*>(&net_sms_id)};
  msg.insert(msg.end(), sms_id_ptr, sms_id_ptr + sizeof(net_sms_id));

  BYTE text_buf[MAX_SMS_TEXT_SIZE]{0};
  size_t text_len{
      std::min(SMS_Text.size(), static_cast<size_t>(MAX_SMS_TEXT_SIZE))};
  std::memcpy(text_buf, SMS_Text.data(), text_len);
  msg.insert(msg.end(), text_buf, text_buf + MAX_SMS_TEXT_SIZE);

  return msg;
}