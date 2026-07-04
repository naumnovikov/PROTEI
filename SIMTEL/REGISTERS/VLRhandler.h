#ifndef VLRHANDLER_H
#define VLRHANDLER_H

#include <mutex>

#include <sqlite3.h>

#include "common_types.h"

class VLR{
private:
    sqlite3 *db;
    WorkingState VLRWorkingState{WorkingState::WORKING};
    std::mutex db_mutex;

    SUCCESS_RESULT setup();
public:
    VLR();
    ~VLR();
    
    bool hasTMSI(TMSI TMSI);
    bool hasMSISDN(std::string_view MSISDN);

    TMSI_And_ENode_b_id getTMSI_And_ENode_b_id_ByMSISDN(std::string_view MSISDN);
    MSISDN getMSISDNByTMSI(TMSI TMSI);
    TMSI getTMSIByMSISDN(std::string_view MSISDN);
    SUCCESS_RESULT add(TMSI TMSI, std::string_view IMSI, std::string_view MSISDN, ENODE_B_ID eNode_B_id);  
    bool updateENodeB(TMSI tmsi, ENODE_B_ID new_enodeb_id);
    int getENodeBIdByTMSI(TMSI tmsi);
    bool hasIMSI(std::string_view IMSI);
    bool removeByTMSI(TMSI tmsi);
};

#endif  // VLRHANDLER_H