#ifndef MOVE_H
#define MOVE_H

#include <vector>

#include "menu.h"

class Move: public Menu
{
private:
    App& app;
    const std::vector<float>& newLocation;
public:
    Move(App& appParam, const std::vector<float>& newLocationParam) : app(appParam), newLocation(newLocationParam){}
    void execute() override;
};

#endif // MOVE_H
