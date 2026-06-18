#ifndef HLRHANDLER_H
#define HLRHANDLER_H

#include <mutex>

#include <sqlite3.h>

#include "common_types.h"

class HLR{
private:
    sqlite3 *db;
    WorkingState HLRWorkingState{WorkingState::WORKING};
    std::mutex db_mutex;

    SUCCESS_RESULT setup();
public:
    HLR();
    ~HLR();

    std::string getMSISDN_ByIMSI(std::string_view IMSI);
    SUCCESS_RESULT add(std::string_view IMSI, std::string_view IMEI, std::string_view MSISDN, MME_ID mmeId);
    MME_ID getMMEid_ByMSISDN(std::string_view MSISDN);
    bool hasMSISDN(std::string_view MSISDN);
    bool updateLocation(std::string_view IMSI, MME_ID new_mme_id);
};

#endif  // HLRHANDLER_H