#ifndef BSINTERFACE_H
#define BSINTERFACE_H

#include "common_types.h"

class BSInterface {
public:
    virtual bool deliverSms(TMSI tmsiDst, std::string_view msisdnSrc,
                            SMS_ID smsId, std::string_view SMS_Text) = 0;
    virtual void sendDeliveryReport(uint32_t tmsi, int smsId, bool success) = 0;
};

#endif // BSINTERFACE_H