#ifndef EIRHANDLER_H
#define EIRHANDLER_H

#include <mutex>

#include <sqlite3.h>

#include "common_types.h"

class EIR{
private:
    sqlite3 *db; 
    WorkingState EIRWorkingState{WorkingState::WORKING};
    std::mutex db_mutex;

    SUCCESS_RESULT setup();
public:
    EIR();
    ~EIR();

    bool IMEILegal(std::string_view IMEI);
    SUCCESS_RESULT add(std::string_view IMEI);
};

#endif  // EIRHANDLER_H