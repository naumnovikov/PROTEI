#ifndef REGISTERSHANDLER_H
#define REGISTERSHANDLER_H

#include "common_types.h"

class HLR;
class VLR;
class EIR;

class RegistersHandler{ 
private:
    HLR *hlr;
    VLR *vlr;
    EIR *eir;
public: 
    RegistersHandler(HLR *hlrParam, VLR *vlrParam, EIR *eirParam){
        hlr = hlrParam;
        vlr = vlrParam;
        eir = eirParam;
    }

    bool vlrHasTMSI(TMSI TMSI);
    bool vlrHasMSISDN(std::string_view MSISDN);
    bool eirIMEILegal(std::string_view IMEI);
    TMSI vlrGetTMSIByMSISDN(std::string_view MSISDN);
    MSISDN vlrGetMSISDNByTMSI(TMSI tmsi);
    TMSI_And_ENode_b_id vlrGetTMSI_And_ENode_b_id_ByMSISDN(std::string_view MSISDN);

    //WARNING: We MUST use eirIMEILegal before adding
    SUCCESS_RESULT addToEIR(std::string_view IMEI);
    SUCCESS_RESULT addToHLR(std::string_view IMSI, std::string_view IMEI, std::string_view MSISDN, MME_ID mmeId);
    SUCCESS_RESULT addToVLR(TMSI TMSI, std::string_view IMSI, std::string_view MSISDN, ENODE_B_ID eNode_B_id);
    bool vlrUpdateENodeB(TMSI tmsi, ENODE_B_ID new_bs_id);
    ENODE_B_ID vlrGetENodeBIdByTMSI(TMSI tmsi);

    MME_ID hlrGetMMEid_ByMSISDN(MSISDN msisdn);

    std::string hlrGetMSISDN_ByIMSI(std::string_view IMSI);
    bool hlrHasMSISDN(std::string_view MSISDN);

    bool isIMSIAlreadyRegistered(std::string_view IMSI);

    bool hlrUpdateLocation(std::string_view IMSI, MME_ID new_mme_id);

    bool vlrRemoveByTMSI(TMSI tmsi);
    
};

#endif  // REGISTERSHANDLER_H