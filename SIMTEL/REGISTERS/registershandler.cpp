#include "registershandler.h"

#include "HLRhandler.h"
#include "VLRhandler.h"
#include "EIRhandler.h"

bool RegistersHandler::vlrHasTMSI(TMSI TMSI){
    return vlr->hasTMSI(TMSI);
}

bool RegistersHandler::vlrHasMSISDN(std::string_view MSISDN){
    return vlr->hasMSISDN(MSISDN);
}

TMSI_And_ENode_b_id RegistersHandler::vlrGetTMSI_And_ENode_b_id_ByMSISDN(std::string_view MSISDN){
    return vlr->getTMSI_And_ENode_b_id_ByMSISDN(MSISDN);
}

TMSI RegistersHandler::vlrGetTMSIByMSISDN(std::string_view MSISDN){
    return vlr->getTMSIByMSISDN(MSISDN);
}

bool RegistersHandler::eirIMEILegal(std::string_view IMEI){
    return eir->IMEILegal(IMEI);
}

MME_ID RegistersHandler::hlrGetMMEid_ByMSISDN(MSISDN msisdn){
    return hlr->getMMEid_ByMSISDN(msisdn);
}

SUCCESS_RESULT RegistersHandler::addToEIR(std::string_view IMEI){
    return eir->add(IMEI);
}

SUCCESS_RESULT RegistersHandler::addToHLR(std::string_view IMSI, std::string_view IMEI, std::string_view MSISDN, MME_ID mmeId){
    return hlr->add(IMSI, IMEI, MSISDN, mmeId);
}

SUCCESS_RESULT RegistersHandler::addToVLR(TMSI TMSI, std::string_view IMSI, std::string_view MSISDN, ENODE_B_ID eNode_B_id){
    return vlr->add(TMSI, IMSI, MSISDN, eNode_B_id);
}

std::string RegistersHandler::vlrGetMSISDNByTMSI(TMSI tmsi){
    return vlr->getMSISDNByTMSI(tmsi);
}

bool RegistersHandler::vlrUpdateENodeB(TMSI tmsi, ENODE_B_ID new_bs_id) {
    return vlr->updateENodeB(tmsi, new_bs_id);
}

ENODE_B_ID RegistersHandler::vlrGetENodeBIdByTMSI(TMSI tmsi) {
    return vlr->getENodeBIdByTMSI(tmsi);
}

std::string RegistersHandler::hlrGetMSISDN_ByIMSI(std::string_view IMSI) {
    return hlr->getMSISDN_ByIMSI(IMSI);
}

bool RegistersHandler::hlrHasMSISDN(std::string_view MSISDN) {
    return hlr->hasMSISDN(MSISDN);
}

bool RegistersHandler::isIMSIAlreadyRegistered(std::string_view IMSI) {
    return vlr->hasIMSI(IMSI); 
}

// Needed when client disconnected and connected again
bool RegistersHandler::hlrUpdateLocation(std::string_view IMSI, MME_ID new_mme_id){ 
    return hlr->updateLocation(IMSI, new_mme_id);
}

bool RegistersHandler::vlrRemoveByTMSI(TMSI tmsi) {
    return vlr->removeByTMSI(tmsi);
}