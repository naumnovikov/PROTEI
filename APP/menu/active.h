#ifndef ACTIVE_H
#define ACTIVE_H

#include "menu.h"

class Active: public Menu
{
private:
    App& app;
    Status newStatus;
public:
    Active(App& appParam, Status newStatusParam) : app(appParam), newStatus(newStatusParam){}
    void execute() override;
};

#endif // ACTIVE_H
