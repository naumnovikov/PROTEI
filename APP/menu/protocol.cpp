#include "protocol.h"

void Protocol::execute(){
    if (app.getProtocol() == newProtocol){
        spdlog::warn("PROTOCOL: already set to requested protocol");
        return;
    }
    app.setProtocol(newProtocol);
    spdlog::info("Protocol changed to {}", (newProtocol == TypeOfProtocol::JSON ? "JSON" : "BINARY"));
    std::cout << "Status set to: ";
    printTypeOfProtocol(newProtocol);
    std::cout << "\n";
}
