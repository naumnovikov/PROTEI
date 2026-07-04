#include "active.h"

void Active::execute() {
    if (ue.getStatus() == newStatus|| ue.getTargetStatus() == newStatus){
        return;
    } 

    ue.setTargetStatus(newStatus); 
    ue.setStatusChanged(true); 
    
    SPDLOG_INFO("Status change requested: {}", (newStatus == Status::ACTIVE ? "ACTIVE" : "NON_ACTIVE"));
}
