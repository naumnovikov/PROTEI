#include "smsc.h"
#include "smsc_db.h"

#include <spdlog/spdlog.h>
#include <thread>

SMS_ID SMSC::processSMS(TMSI TMSI_src, std::string_view MSISDN_dst, std::string_view SMS_Text) {
    if (this == nullptr) {
        SPDLOG_ERROR("CRITICAL: SMSC object (this) is nullptr! MME failed to initialize SMSC.");
        return SMSC_ERROR_CODE;
    }

    SMS_ID sms_id{SMSIdGenerator::generate()};
    while (smsc_db->hasSMSId(sms_id)) {
        sms_id = SMSIdGenerator::generate();
    }

    if (smsc_db->add(TMSI_src, MSISDN_dst, SMS_Text, sms_id) != OK_CODE) {
        SPDLOG_ERROR("SMSC: failed to store SMS");
        return SMSC_ERROR_CODE;
    }

    // TTL for SMS
    const int TTL_SECONDS{30};
    std::thread([this, sms_id, TMSI_src, TTL_SECONDS]() {
        std::this_thread::sleep_for(std::chrono::seconds(TTL_SECONDS));
        if (smsc_db->hasSMSId(sms_id)) {
            smsc_db->remove(sms_id);
            if (mme) {
                mme->sendDeliveryReportToSource(TMSI_src, sms_id, false);
            }
            SPDLOG_WARN("SMSC: SMS {} expired, removed, notified src {}", sms_id, TMSI_src);
        }
    }).detach();

    SPDLOG_INFO("SMSC: SMS {} registered, TTL={}s", sms_id, TTL_SECONDS);
    return sms_id;
}

void SMSC::removeSMS(SMS_ID sms_id) {
    if (smsc_db != nullptr) {
        smsc_db->remove(sms_id);
    } else {
        SPDLOG_ERROR("SMSC: Failed to remove SMS {}. Database pointer is null!", sms_id);
    }
}

TMSI SMSC::getSenderTMSI(SMS_ID sms_id) {
    return smsc_db->getSenderTMSI(sms_id); 
}