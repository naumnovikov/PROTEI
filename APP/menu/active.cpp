#include "active.h"

void Active::execute(){
    if (app.getStatus() == newStatus){
        spdlog::warn("ACTIVE: status already set to the requested value");
        return;
    }
    app.setStatus(newStatus);
    spdlog::info("Status changed to {}", (newStatus == Status::ACTIVE ? "ACTIVE" : "NON_ACTIVE"));
    printStatus(newStatus);
    std::cout << "\n";
}


