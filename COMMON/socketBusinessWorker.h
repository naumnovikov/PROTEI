#ifndef SOCKETBUSINESSWORKER_H
#define SOCKETBUSINESSWORKER_H

#include <exception>

#include "sock.h"

// TO DO:
//  - decrease quantity of arguments by using struct, std::tuple

class SocketBusinessWorker {
 private:
  void push4BytesInBE(BYTE_VECTOR& msg, uint32_t data) const;
  uint32_t decodeUint32FromBEBytes(const BYTE* data) const noexcept;

 public:
  SUCCESS_RESULT sendDeliveryReply(FD sock, TMSI tmsi, SMS_ID sms_id,
                                   DeliveryStatus status) const;

  SUCCESS_RESULT sendTMSIFromBSToUE(TMSI TMSI, FD sock, const char* client_ip,
                                    PORT client_port) const;

  void encodeFloatToBEBytes(float f, BYTE* out) const noexcept;

  float decodeFloatFromBEBytes(const BYTE* data) const noexcept;

  SUCCESS_RESULT decodeMessageMsg(ENODE_B_ID eNode_b_receiver_id,
                                  TMSI& TMSI_src, MSISDN& MSISDN_dst,
                                  SMS_ID& smsId, std::string& SMS_Text,
                                  BYTE_VECTOR& msg_buf, const char* client_ip,
                                  PORT client_port) const;

  int decodeIntFromBEBytes(const BYTE* data) const noexcept;

  SUCCESS_RESULT decodeRequestMsg(ENODE_B_ID eNode_b_receiver_id, IMSI& ue_imsi,
                                  position_vector& ue_location,
                                  BYTE_VECTOR& msg_buf, const char* client_ip,
                                  PORT client_port) const;

  SUCCESS_RESULT decodeAttachMsg(ENODE_B_ID eNode_b_receiver_id, IMSI& ue_imsi,
                                 IMEI& ue_imei, ENODE_B_ID& eNode_B_id,
                                 BYTE_VECTOR& msg_buf, const char* client_ip,
                                 PORT client_port) const;

  SUCCESS_RESULT sendHandoverCommand(FD sock, const IPv4& bs_ip, PORT bs_port,
                                     const char* client_ip,
                                     PORT client_port) const;

  BYTE_VECTOR encodeDeliveryMsg(TMSI tmsiDst, std::string_view msisdnSrc,
                                SMS_ID smsId, std::string_view SMS_Text) const;

  SUCCESS_RESULT sendCoefReply(float coef, ENODE_B_ID eNodeB_id, FD client_fd,
                               const char* client_ip, PORT client_port) const;
};

#endif  // SOCKETBUSINESSWORKER_H