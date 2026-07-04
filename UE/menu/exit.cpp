#include "exit.h"

void Exit::execute() {
    SPDLOG_INFO("Exit command received, shutting down");
    
    ue.stopReceiver(); 
}
