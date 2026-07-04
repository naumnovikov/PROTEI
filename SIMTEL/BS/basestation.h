#ifndef BASESTATION_H
#define BASESTATION_H

#include <memory>
#include <unordered_map>
#include <mutex>

#include "threadpool.h"
#include "socketBusinessWorker.h"
#include "networkaddress.h"
#include "mmeinterface.h"
#include "BSInterface.h"

struct UEContext {
    TMSI tmsi;
    std::shared_ptr<Sock> socket;
    
    UEContext(uint32_t t, std::shared_ptr<Sock> s) : tmsi(t), socket(std::move(s)) {}
};

using UEContainer = std::unordered_map<TMSI, UEContext>;

class BS : public BSInterface{
private:
    ENODE_B_ID eNode_B_id;
    NetworkAddress BSAddress;
    position_vector position;
    BS_RADIUS serviceRadius;
    WorkingState eNode_B_state{WorkingState::WORKING};
    SocketBusinessWorker socketBusinessWorker;
    MmeInterface* mme; 
    UEContainer ueContainer;
    std::mutex containerMutex;
    Sock listenerSocket;

    void processClient(std::shared_ptr<Sock> client_sock, ThreadPool& pool, const char* client_ip, PORT client_port);
    void processClients(ThreadPool& pool);
public:
    inline BS(ENODE_B_ID eNode_B_id_param, MmeInterface* mme_param, const IPv4& ip, PORT port, position_vector position_param, BS_RADIUS serviceRadius_param){ // position_vector нужно перемещать так как выше он нигде не используется
      eNode_B_id = eNode_B_id_param;
      mme = mme_param;
      BSAddress.initialize(ip, port);
      position = std::move(position_param);
      serviceRadius = serviceRadius_param;
    } 

    void work();
    void removeUE(TMSI tmsi);
    bool deliverSms(TMSI tmsiDst, std::string_view msisdnSrc, SMS_ID smsId, std::string_view SMS_Text) override;
    void sendDeliveryReport(TMSI tmsi, SMS_ID smsId, bool success) override;
    CONNECTION_COEF calculateConnectionCoef(position_vector UE_location);

    std::shared_ptr<Sock> getSocketByTMSI(TMSI tmsi);   
    inline ENODE_B_ID getId() const noexcept{ return eNode_B_id;}
    inline position_vector getPosition() const noexcept{ return position; }
    inline BS_RADIUS getRadius() const noexcept{ return serviceRadius; }
    inline IPv4 getIp() const noexcept{ return BSAddress.getIpString(); }
    inline PORT getPort() const noexcept{ return BSAddress.getPort(); }

};

#endif  // BASESTATION_H

