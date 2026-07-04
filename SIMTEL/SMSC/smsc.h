#ifndef SMSC_H
#define SMSC_H

#include <memory>

#include "mmeinterface.h"
#include "smsc_db.h"
#include "SMSIdGenerator.h"

class SMSC_db;

class SMSC{
private:
    MmeInterface* mme; 
    std::unique_ptr<SMSC_db> smsc_db;
public:
    inline SMSC() : smsc_db(std::make_unique<SMSC_db>()){}

    SMS_ID processSMS(TMSI TMSI_src, std::string_view MSISDN_dst, std::string_view SMS_Text); 
    void setMME(MmeInterface* m) { mme = m; }
    void removeSMS(SMS_ID sms_id);
    TMSI getSenderTMSI(SMS_ID sms_id);
};

#endif  // SMSC_H
