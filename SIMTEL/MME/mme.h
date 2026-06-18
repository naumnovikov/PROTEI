#ifndef MME_H
#define MME_H

#include <map>

#include <spdlog/spdlog.h>

#include "mmeinterface.h"
#include "TMSIGenerator.h"
#include "MSISDNGenerator.h"
#include "registershandler.h"
#include "basestation.h"
#include "smsc.h"
#include "common_types.h"

class MME; 
using MMEContainer = std::unordered_map<MME_ID, MME>;

class MME : public MmeInterface{
private:
    std::unique_ptr<RegistersHandler> regHandler;
    std::unordered_map<ENODE_B_ID, BS*> baseStationsPtrsContainer;
    SMSC* smsc;
    const MMEContainer* mmeContainer;
    MME_ID mme_id;

    TMSI generateTMSI();
public:
    void registerBaseStation(BS* bs) {
        if (bs != nullptr) {
            baseStationsPtrsContainer[bs->getId()] = bs;
            SPDLOG_INFO("MME: Registered BS ID {}", bs->getId());
        }
    }
    TMSI_dst_And_MSISDN_src getTMSI_dst_And_MSISDN_src(ENODE_B_ID eNode_B_id, TMSI tmsiSrc, const MSISDN& msisdnDst, const std::string& text, SMS_ID smsId);
    SUCCESS_RESULT forwardSmsFromAnotherMME(std::string_view MSISDN_dst, std::string_view MSISDN_src, std::string_view SMS_Text, SMS_ID sms_id);
    BS* getBSById(ENODE_B_ID bs_id) override;


    void setRegistersHandler(std::unique_ptr<RegistersHandler> handler);
    inline void setSMSC(SMSC* s) noexcept{ smsc = s; }

    TMSI onAttachRequest(ENODE_B_ID eNode_B_id, const IMSI& imsi, const IMEI& imei) override;
    DeliveryStatus onSmsSend(TMSI TMSI_src, std::string_view MSISDN_dst, std::string_view SMS_Text) override;  
    int getBestBSForLocation(const position_vector& location, ENODE_B_ID current_bs_id) const override;
    void initiateHandover(TMSI tmsi, ENODE_B_ID target_bs_id) override;
    void sendDeliveryReportToSource(TMSI tmsi_src, SMS_ID sms_id, bool success) override;
    void onDeliveryReport(TMSI tmsi, SMS_ID sms_id, bool success) override;
    void onClientDisconnect(TMSI tmsi) override;
    void sendSubmissionAckToSource(TMSI tmsi_src, SMS_ID sms_id) override;
};

#endif  // MME_H