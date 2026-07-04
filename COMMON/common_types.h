#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <array>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <vector>

enum class WorkingState { WORKING, NOT_WORKING };

enum class Status { ACTIVE, NON_ACTIVE };
inline void printStatus(Status status) noexcept {
  if (status == Status::ACTIVE) {
    std::cout << "ACTIVE" << std::endl;
    return;
  }
  std::cout << "NON_ACTIVE" << std::endl;
}

using TMSI = uint32_t;
using SMS_ID = int;
using position_vector = std::vector<float>;
using SUCCESS_RESULT = int;
using MSISDN = std::string;
using ENODE_B_ID = int;
using TMSI_And_ENode_b_id = std::tuple<TMSI, ENODE_B_ID>;
using IPv4 = std::string;
using IMEI = std::string;
using IMSI = std::string;
using PORT = uint16_t;
using json = nlohmann::json;
using BS_RADIUS = float;
using JSON_FILE = std::ifstream;
using BYTE = uint8_t;
using FD = int;
using DeliveryStatus = char;
using BYTE_VECTOR = std::vector<BYTE>;
using CONNECTION_COEF = float;
using MME_ID = int;
using TMSI_dst_And_MSISDN_src = std::tuple<TMSI, MSISDN>;
using MSISDN_ARRAY = std::array<BYTE, 15>;
using SMS_TEXT_ARRAY = std::array<BYTE, 140>;
using TOKENS_VECTOR = std::vector<std::string>;
using COMMAND = std::string;

constexpr int OK_CODE{0};
constexpr int ERROR_CODE{-1};
constexpr int UNDEFINED_SOCKET{-1};
constexpr int TCP_VALUE{0};

constexpr uint16_t PORT_EXAMPLE{3000};
const IPv4 IPv4_EXAMPLE{"127.0.0.1"};
constexpr int MIN_LAST_IP_BYTE{1};
constexpr int MAX_LAST_IP_BYTE{253};
constexpr int IANA_REGISTRED_PORTS_MIN{1024};
constexpr int IANA_REGISTRED_PORTS_MAX{49151};

constexpr std::size_t num_threads{20};
constexpr uint32_t size_len{4};
constexpr uint32_t FLOAT_SIZE{4};
constexpr uint32_t float_len{4};
constexpr uint32_t EMPTY_LEN{0};
constexpr uint32_t MAX_REST_LEN{65536};
constexpr int IMEI_SIZE{15};
constexpr int MAX_IMSI_SIZE{15};
constexpr int MIN_IMSI_SIZE{1};
constexpr std::size_t COORDINATES_QUANTITY{3};

constexpr uint32_t ONE_BYTE_SHIFT{8};
constexpr uint32_t THREE_BYTES{24};
constexpr uint32_t TWO_BYTES{16};
constexpr uint32_t FULL_ONE_BYTE_MASK{0xFF};

constexpr int MAX_SMS_TEXT_SIZE{140};

constexpr std::size_t MSG_TYPE_INDEX{0};
constexpr int MSG_TYPE_BYTES_QUANTITY{1};

constexpr std::size_t SMSID_MESSAGE_START{20};

// COEF REPLY MSG
constexpr std::size_t COEF_REPLY_MSG_SIZE{8};
constexpr std::size_t COEF_BYTES_QUANTITY_IN_COEF_REPLY_MSG{4};
constexpr std::size_t ENODE_B_ID_BYTES_QUANTITY_IN_COEF_REPLY_MSG{4};

// REQUEST MSG
constexpr int REQUEST_MSG_SIZE{28};
constexpr std::size_t IMSI_REQUEST_START{1};
constexpr std::size_t IMSI_REQUEST_END{16};
constexpr std::size_t COORDINATES_REQUEST_START{16};

// TMSI FROME BS TO UE MSG
constexpr int TMSI_BYTES_QUANTITY_IN_TMSI_FROM_BS_TO_UE_MSG{4};

// ATTACH MSG
constexpr std::size_t IMSI_ATTACH_START{1};
constexpr std::size_t IMSI_ATTACH_END{16};
constexpr std::size_t IMEI_ATTACH_START{16};
constexpr std::size_t IMEI_ATTACH_END{31};
constexpr std::size_t ENODEB_ID_ATTACH_START{31};

// MESSAGE MSG
constexpr std::size_t TMSI_MESSAGE_START{1};
constexpr std::size_t MSISDN_MESSAGE_START{5};
constexpr std::size_t MSISDN_MESSAGE_END{20};
constexpr std::size_t SMS_MESSAGE_START{24};
constexpr std::size_t SMS_MESSAGE_END{163};

// DELIVERY REPLY MSG
constexpr int TMSI_BYTES_QUANTITY_IN_DELIVERY_REPLY_MSG{4};
constexpr int SMS_ID_BYTES_QUANTITY_IN_DELIVERY_REPLY_MSG{4};
constexpr int DELIVERY_STATUS_BYTES_QUANTITY_IN_DELIVERY_REPLY_MSG{4};

// HANDOVER COMMAND MSG
constexpr int IPv4_BYTES_QUANTITY_IN_HANDOVER_COMMAND_MSG{4};
constexpr int PORT_BYTES_QUANTITY_IN_HANDOVER_COMMAND_MSG{2};

// DELIVERY MSG
constexpr int TMSI_BYTES_QUANTITY_IN_DELIVERY_MSG{4};
constexpr int MSISDN_BYTES_QUANTITY_IN_DELIVERY_MSG{15};
constexpr int SMS_ID_BYTES_QUANTITY_IN_DELIVERY_MSG{4};
constexpr int SMS_TEXT_BYTES_QUANTITY_IN_DELIVERY_MSG{140};

// BS
constexpr float ERROR_CONNECTION_COEF{-1.0f};
constexpr int ONE_BYTE{1};
constexpr size_t SMS_FULL_SIZE{1 + 4 + 15 + 4 + 140};
constexpr size_t UPDATE_SIZE{1 + 12};
constexpr int NO_OTHER_BS_CODE{-1};
constexpr size_t ACK_SIZE{1 + 4 + 4 + 1};
constexpr int INT_SIZE{4};
constexpr int STATUS_BYTE_START{9};
constexpr BYTE SUCCESS_BYTE{1};
constexpr int UE_RESPONCE_TIMEOUT_SEC{120};
constexpr int CONNECTTION_TIMEOUT_SEC{240};
constexpr int CONNECTIONS_QUEUE_SIZE_LIMIT{5};

// MME
constexpr uint32_t MME_ERROR_CODE{0};
constexpr TMSI_And_ENode_b_id ERROR_TUPLE{0, -1};
const std::string ERROR_MSISDN{""};
const TMSI_dst_And_MSISDN_src TMSI_dst_And_MSISDN_src_ERROR_TUPLE{
    0u, std::string("")};
constexpr SMS_ID INVALID_SMS_ID{0};
constexpr DeliveryStatus DELIVERY_STATUS_ERROR{0};
constexpr DeliveryStatus DELIVERY_STATUS_SUCCESS{1};
constexpr int MMEID_ERROR{-1};
constexpr int SUBMISSION_ACK_SIZE{5};
constexpr TMSI INVALID_TMSI{0};
constexpr float SERVICE_COEF_MIN{0.0f};

// SMSC
constexpr int SMSC_ERROR_CODE{0};

// UE
constexpr std::size_t SMS_MIN_TOKENS_QUANTITY{3};
constexpr std::size_t THIRD_SMS_TOKEN_INDEX{2};
constexpr std::size_t MESSAGE_MSG_HEADER_SIZE{23};
constexpr std::size_t MESSAGE_MSG_SIZE{1 + MESSAGE_MSG_HEADER_SIZE + 140};
constexpr std::size_t TMSI_SIZE{4};
constexpr std::size_t MSISDN_SIZE{15};
constexpr std::size_t SMS_ID_SIZE{4};
constexpr int ACTIVE_COMMAND_MIN_TOKENS{2};
constexpr int MOVE_COMMAND_MIN_TOKENS{2};
constexpr int MOVE_COMMAND_MAX_TOKENS{4};
constexpr int PROTOCOL_COMMAND_MIN_TOKENS{2};
constexpr std::size_t M_MSISDN_OFFSET{5};
constexpr std::size_t M_SMS_ID_OFFSET{20};
constexpr std::size_t m_MSG_SIZE{9};
constexpr std::size_t T_MSG_SIZE{4};
constexpr std::size_t COEF_SIZE{4};
constexpr std::size_t H_MSG_SIZE{6};
constexpr std::size_t IP_SIZE{4};
constexpr std::size_t PORT_SIZE{2};
constexpr std::size_t DELIVERY_REPLY_MSG_SIZE{1};
constexpr std::size_t S_MSG_SIZE{4};
constexpr std::size_t MAX_MOVE_TOKENS_QUANTITY{4};
constexpr char PADDING_ZERO{'0'};
constexpr std::size_t UINT32_T_SIZE{4};
constexpr uint32_t ERROR_ENODE_B_ID{0};
constexpr std::size_t ATTACH_MSG_SIZE{1 + 15 + 15 + 4};

#endif  // COMMON_TYPES_H