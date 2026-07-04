#ifndef MMEINTERFACE_H
#define MMEINTERFACE_H

#include "common_types.h"

class BS;

class MmeInterface {
public:
    virtual TMSI onAttachRequest(ENODE_B_ID eNode_B_id, const IMSI& imsi, const IMEI& imei) = 0;
    virtual DeliveryStatus onSmsSend(TMSI TMSI_src, std::string_view MSISDN_dst, std::string_view SMS_Text) = 0;
    virtual int getBestBSForLocation(const position_vector& location, ENODE_B_ID current_bs_id) const = 0;
    virtual void initiateHandover(TMSI tmsi, ENODE_B_ID target_bs_id) = 0;
    virtual void sendDeliveryReportToSource(TMSI tmsi_src, SMS_ID sms_id, bool success) = 0;
    virtual BS* getBSById(ENODE_B_ID bs_id) = 0;
    virtual void onDeliveryReport(TMSI tmsi, SMS_ID sms_id, bool success) = 0;
    virtual void onClientDisconnect(TMSI tmsi) = 0;
    virtual void sendSubmissionAckToSource(TMSI tmsi_src, SMS_ID sms_id) = 0;
};

#endif // MMEINTERFACE_H