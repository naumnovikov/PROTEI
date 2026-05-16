#ifndef MENU_H
#define MENU_H

#include <spdlog/spdlog.h>

#include "app.h"

class Menu{
public:
    virtual void execute() = 0;
};

#endif // MENU_H
