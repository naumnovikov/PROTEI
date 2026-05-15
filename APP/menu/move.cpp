#include "move.h"

void Move::execute(){
    for (size_t i{0}; i < newLocation.size(); ++i){
        app.getLocationForMoving().at(i) = newLocation.at(i);
    }
    spdlog::info("Location changed to: {}, {}, {}",
                 newLocation.size()>0 ? std::to_string(newLocation[0]) : "?",
                 newLocation.size()>1 ? std::to_string(newLocation[1]) : "?",
                 newLocation.size()>2 ? std::to_string(newLocation[2]) : "?");
    std::cout << "Location set to: ";
    for (const auto& coordinate : app.getLocationForMoving()){
        std::cout << coordinate << ' ';
    }
    std::cout << "\n";
}
