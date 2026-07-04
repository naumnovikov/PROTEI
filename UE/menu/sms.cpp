#include "sms.h"

#include <algorithm>

#include <sys/socket.h>
#include <netinet/in.h>

void SMS::execute() {
    BYTE_VECTOR msg;
    msg.reserve(MESSAGE_MSG_SIZE); 

    msg.push_back('M');

    TMSI tmsi_net{htonl(ue.getTmsi())};
    msg.insert(msg.end(), reinterpret_cast<BYTE*>(&tmsi_net),
               reinterpret_cast<BYTE*>(&tmsi_net) + TMSI_SIZE);

    MSISDN_ARRAY dst_buf{};
    std::size_t copy_len{std::min(dst_msisdn.size(), MSISDN_SIZE)};
    std::copy(dst_msisdn.begin(), dst_msisdn.begin() + copy_len, dst_buf.begin());
    msg.insert(msg.end(), dst_buf.begin(), dst_buf.end());

    SMS_ID sms_id_zero{0};
    msg.insert(msg.end(), reinterpret_cast<BYTE*>(&sms_id_zero),
               reinterpret_cast<BYTE*>(&sms_id_zero) + SMS_ID_SIZE);

    SMS_TEXT_ARRAY text_buf{};
    copy_len = std::min(sms_text.size(), size_t(140));
    std::copy(sms_text.begin(), sms_text.begin() + copy_len, text_buf.begin());
    msg.insert(msg.end(), text_buf.begin(), text_buf.end());

    FD fd{ue.getFd()};
    if (fd < 0) {
        throw std::runtime_error("Invalid socket: UE is not connected to the network");
    }
    ssize_t sent{send(fd, msg.data(), msg.size(), TCP_VALUE)};
    if (sent == ERROR_CODE) {
        SPDLOG_ERROR("SMS send failed: {}", strerror(errno));
    } else {
        SPDLOG_INFO("SMS sent to {} ({} bytes)", dst_msisdn, sent);
    }
}