#include "mme.h"

#include <mutex>
#include <cctype>

TMSI MME::generateTMSI(){ 
    bool freeTMSIGenerated{false};
    TMSI tmsi_buffer;
    while (!freeTMSIGenerated){
        tmsi_buffer = TMSIGenerator::generate();
        if (!regHandler->vlrHasTMSI(tmsi_buffer)){ 
            freeTMSIGenerated = true;
        }
    }

    return tmsi_buffer;
}

TMSI MME::onAttachRequest(ENODE_B_ID eNode_B_id, const IMSI& imsi, const IMEI& imei) {
    // static to avoid data race when working with 
    // registers, which are general
    static std::mutex attach_mutex; 
    std::lock_guard<std::mutex> lock(attach_mutex);

    //To normalize string
    auto sanitize{[](const std::string& str) {
        std::string res;
        for (char c : str) {
            if (std::isalnum(c)){
                res += c;
            }
        }
        return res;
    }};

    IMSI clean_imsi{sanitize(imsi)};
    IMEI clean_imei{sanitize(imei)};

    if (!regHandler->eirIMEILegal(clean_imei)) [[unlikely]]{
        return MME_ERROR_CODE;
    }

    if (regHandler->isIMSIAlreadyRegistered(clean_imsi)) {
        MSISDN msisdn{regHandler->hlrGetMSISDN_ByIMSI(clean_imsi)};
        TMSI_And_ENode_b_id tmsi_data{regHandler->vlrGetTMSI_And_ENode_b_id_ByMSISDN(msisdn)};
        
        if (tmsi_data != ERROR_TUPLE) {
            TMSI existing_tmsi{std::get<0>(tmsi_data)};
            ENODE_B_ID saved_enodeb_id{std::get<1>(tmsi_data)};

            if (saved_enodeb_id == eNode_B_id) {
                SPDLOG_INFO("MME: Reconnect/Handover complete for IMSI: {} on BS {}", clean_imsi, eNode_B_id);
                return existing_tmsi;
            } else {
                 SPDLOG_INFO("MME: Reconnect for IMSI: {} on new BS {}. Updating VLR.", clean_imsi, eNode_B_id);
                 regHandler->vlrUpdateENodeB(existing_tmsi, eNode_B_id);
                 return existing_tmsi;
            }
        }
    }

    MSISDN msisdn{regHandler->hlrGetMSISDN_ByIMSI(clean_imsi)};
    bool is_existing_subscriber{!msisdn.empty()};

    if (!is_existing_subscriber) {
        // New sim for new UE
        msisdn = MSISDNGenerator::generate(); 
        
        while (regHandler->vlrHasMSISDN(msisdn)) [[unlikely]]{
            SPDLOG_WARN("MME: MSISDN collision for {}, generating a new one...", msisdn);
            msisdn = MSISDNGenerator::generate();
        }
        
        if (regHandler->addToEIR(clean_imei) != OK_CODE) [[unlikely]]{
            return MME_ERROR_CODE;
        }
        if (regHandler->addToHLR(clean_imsi, clean_imei, msisdn, mme_id) != OK_CODE) [[unlikely]]{ 
            return MME_ERROR_CODE;
        }
        SPDLOG_INFO("MME: New subscriber registered. IMSI: {}, MSISDN: {}", clean_imsi, msisdn);
    } else {
        SPDLOG_INFO("MME: Existing subscriber attached. IMSI: {}, Restored MSISDN: {}", clean_imsi, msisdn);
        regHandler->hlrUpdateLocation(clean_imsi, mme_id);
    }
    TMSI new_tmsi{generateTMSI()}; 

    if (regHandler->addToVLR(new_tmsi, clean_imsi, msisdn, eNode_B_id) != OK_CODE) [[unlikely]]{ 
        return MME_ERROR_CODE;
    }

    return new_tmsi;
}

void MME::onClientDisconnect(TMSI tmsi){
    regHandler->vlrRemoveByTMSI(tmsi);
}

int MME::getBestBSForLocation(const position_vector& location, ENODE_B_ID current_bs_id) const {    
    ENODE_B_ID best_bs_id{NO_OTHER_BS_CODE};
    CONNECTION_COEF best_coef{ERROR_CONNECTION_COEF};

    for (const auto& [bs_id, bs] : baseStationsPtrsContainer) {

        float coef{bs->calculateConnectionCoef(location)};

        if (coef > SERVICE_COEF_MIN && coef > best_coef) {
            best_coef = coef;
            best_bs_id = bs_id;
        }
    }
    
    SPDLOG_INFO("MME DEBUG: Selected BS: {}", best_bs_id);
    return best_bs_id;
}

TMSI_dst_And_MSISDN_src MME::getTMSI_dst_And_MSISDN_src(ENODE_B_ID eNode_B_id, TMSI tmsiSrc, const MSISDN& msisdnDst, const std::string& text, SMS_ID smsId) {
    if (regHandler->vlrHasMSISDN(msisdnDst)) {
        return {regHandler->vlrGetTMSIByMSISDN(msisdnDst), regHandler->vlrGetMSISDNByTMSI(tmsiSrc)};
    }
    return TMSI_dst_And_MSISDN_src_ERROR_TUPLE;
}

SUCCESS_RESULT MME::forwardSmsFromAnotherMME(std::string_view MSISDN_dst, std::string_view MSISDN_src, std::string_view SMS_Text, SMS_ID sms_id) {
    auto tmsi_and_enodeb{regHandler->vlrGetTMSI_And_ENode_b_id_ByMSISDN(MSISDN_dst)};
    if (tmsi_and_enodeb == ERROR_TUPLE) [[unlikely]] {
        SPDLOG_ERROR("forwardSmsFromAnotherMME: recipient {} not in VLR", MSISDN_dst);
        return ERROR_CODE;
    }

    TMSI tmsi_dst{std::get<0>(tmsi_and_enodeb)};
    ENODE_B_ID enodeb_id{std::get<1>(tmsi_and_enodeb)};

    auto bs_it{baseStationsPtrsContainer.find(enodeb_id)};
    if (bs_it == baseStationsPtrsContainer.end()) [[unlikely]] {
        SPDLOG_ERROR("forwardSmsFromAnotherMME: eNodeB {} not found", enodeb_id);
        return ERROR_CODE;
    }

    bs_it->second->deliverSms(tmsi_dst, MSISDN_src, sms_id, SMS_Text);
    SPDLOG_INFO("forwardSmsFromAnotherMME: SMS {} delivered to TMSI {} via eNodeB {}", sms_id, tmsi_dst, enodeb_id);
    return OK_CODE;
}

DeliveryStatus MME::onSmsSend(TMSI TMSI_src, std::string_view MSISDN_dst, std::string_view SMS_Text) {
    std::string clean_msisdn_dst{MSISDN_dst}; 

    SMS_ID sms_id {smsc->processSMS(TMSI_src, clean_msisdn_dst, SMS_Text)};
    if (sms_id == INVALID_SMS_ID) [[unlikely]] {
        SPDLOG_ERROR("SMSC failed to register SMS");
        return DELIVERY_STATUS_ERROR;
    }

    sendSubmissionAckToSource(TMSI_src, sms_id);

    std::string msisdn_src{regHandler->vlrGetMSISDNByTMSI(TMSI_src)};
    if (msisdn_src.empty()) [[unlikely]]{
        SPDLOG_ERROR("Cannot find MSISDN for TMSI_src {}", TMSI_src);
        smsc->removeSMS(sms_id);
        return DELIVERY_STATUS_ERROR;
    }

    auto tmsi_dst_and_enodeb{regHandler->vlrGetTMSI_And_ENode_b_id_ByMSISDN(clean_msisdn_dst)};
    
    if (tmsi_dst_and_enodeb != ERROR_TUPLE) {
        TMSI tmsi_dst{std::get<0>(tmsi_dst_and_enodeb)};
        ENODE_B_ID enodeb_id_dst{std::get<1>(tmsi_dst_and_enodeb)};
        auto bs_it{baseStationsPtrsContainer.find(enodeb_id_dst)};
        
        if (bs_it != baseStationsPtrsContainer.end()) {
            bool success{bs_it->second->deliverSms(tmsi_dst, msisdn_src, sms_id, SMS_Text)};
            if (success) {
                SPDLOG_INFO("SMS {} delivered...", sms_id);
                return DELIVERY_STATUS_SUCCESS;
            } else {
                SPDLOG_ERROR("SMS {} delivery FAILED at BS {}", sms_id, enodeb_id_dst);
                return DELIVERY_STATUS_ERROR;
            }
            SPDLOG_INFO("SMS {} delivered to MSISDN {} via eNodeB {}", sms_id, clean_msisdn_dst, enodeb_id_dst);
            return DELIVERY_STATUS_SUCCESS;
        }
    }

    MME_ID mme_id{regHandler->hlrGetMMEid_ByMSISDN(clean_msisdn_dst)};
    if (mme_id == MMEID_ERROR) {
        SPDLOG_WARN("MSISDN {} does not exist in HLR", clean_msisdn_dst);
        
        smsc->removeSMS(sms_id); 
        
        sendDeliveryReportToSource(TMSI_src, sms_id, false);
        return DELIVERY_STATUS_ERROR;
    }

    if (mme_id != MMEID_ERROR && mmeContainer && mmeContainer->count(mme_id)) {
        //TO DO:
        // - implement sending SMS to UE from another MME
        return DELIVERY_STATUS_ERROR; 
    } else {
        SPDLOG_INFO("SMS {} stored in SMSC for offline recipient {}", sms_id, clean_msisdn_dst);
        return DELIVERY_STATUS_ERROR;
    }
}

void MME::sendSubmissionAckToSource(TMSI tmsi_src, SMS_ID sms_id) {
    ENODE_B_ID enodeb_id{regHandler->vlrGetENodeBIdByTMSI(tmsi_src)};
    if (enodeb_id == NO_OTHER_BS_CODE){
        return;
    }

    auto it{baseStationsPtrsContainer.find(enodeb_id)};
    if (it != baseStationsPtrsContainer.end()) {
        auto client_sock{it->second->getSocketByTMSI(tmsi_src)};
        if (client_sock) {
            BYTE_VECTOR ack(SUBMISSION_ACK_SIZE);
            ack[0] = 'S';
            uint32_t net_sms_id{htonl(static_cast<uint32_t>(sms_id))};
            std::memcpy(ack.data() + ONE_BYTE, &net_sms_id, SMS_ID_SIZE);
            send(client_sock->getFd(), ack.data(), ack.size(), TCP_VALUE);
        }
    }
}

void MME::initiateHandover(TMSI tmsi, ENODE_B_ID target_bs_id) {
    if (!regHandler->vlrUpdateENodeB(tmsi, target_bs_id)) {
        SPDLOG_ERROR("Handover failed: cannot update VLR for TMSI {}", tmsi);
        return;
    }
    SPDLOG_INFO("Handover completed: TMSI {} now served by BS {}", tmsi, target_bs_id);
}

void MME::sendDeliveryReportToSource(TMSI tmsi_src, SMS_ID sms_id, bool success) {
    ENODE_B_ID enodeb_id{regHandler->vlrGetENodeBIdByTMSI(tmsi_src)};
    if (enodeb_id == -1) {
        SPDLOG_ERROR("Cannot find source BS for TMSI {}", tmsi_src);
        return;
    }
    auto it{baseStationsPtrsContainer.find(enodeb_id)};
    if (it != baseStationsPtrsContainer.end()) {
        it->second->sendDeliveryReport(tmsi_src, sms_id, success);
        SPDLOG_INFO("Delivery report sent to TMSI {} with status {}", tmsi_src, success);
    } else {
        SPDLOG_ERROR("Source BS {} not found", enodeb_id);
    }
}

void MME::setRegistersHandler(std::unique_ptr<RegistersHandler> handler) {
    regHandler = std::move(handler);
}

BS* MME::getBSById(ENODE_B_ID bs_id) {
    auto it{baseStationsPtrsContainer.find(bs_id)};
    if (it != baseStationsPtrsContainer.end()) {
        return it->second; 
    }
    return nullptr;
}

void MME::onDeliveryReport(TMSI tmsi_dst, SMS_ID sms_id, bool success) {
    SPDLOG_INFO("MME received delivery report from recipient TMSI={}, SMS_ID={}, success={}", tmsi_dst, sms_id, success);

    TMSI tmsi_src{smsc->getSenderTMSI(sms_id)}; 

    if (tmsi_src != INVALID_TMSI) {
        sendDeliveryReportToSource(tmsi_src, sms_id, success);
    } else [[unlikely]]{
        SPDLOG_WARN("Cannot send delivery report: unknown sender for SMS_ID {}", sms_id);
    }

    if (success) {
        smsc->removeSMS(sms_id);
    }
}
