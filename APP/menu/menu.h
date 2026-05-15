#ifndef MENU_H
#define MENU_H

#include "app.h"
#include <spdlog/spdlog.h>

class Menu{
public:
    virtual void execute() = 0;
    void printMenu();
};

#endif // MENU_H
