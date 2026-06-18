#ifndef SMSC_DB_H
#define SMSC_DB_H

#include <mutex>

#include <sqlite3.h>

#include "common_types.h"
#include "smsc.h"

class SMSC_db{
private:
    std::mutex db_mutex;
    sqlite3 *db; 
    WorkingState SMSC_db_WorkingState{WorkingState::WORKING};
    
    SUCCESS_RESULT setup();
public:
    SMSC_db();
    ~SMSC_db();

    SUCCESS_RESULT add(TMSI TMSI_src, std::string_view MSISDN_dst, std::string_view SMS_Text, SMS_ID sms_id);
    bool hasSMSId(SMS_ID sms_id);
    SUCCESS_RESULT remove(SMS_ID sms_id);

    uint32_t getSenderTMSI(SMS_ID sms_id);
};

#endif  // SMSC_DB_H