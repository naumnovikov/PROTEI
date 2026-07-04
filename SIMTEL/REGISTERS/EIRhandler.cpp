#include "EIRhandler.h"

#include "spdlog/spdlog.h"

SUCCESS_RESULT EIR::setup() {
    const char* sql{"CREATE TABLE IF NOT EXISTS eir ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "IMEI TEXT NOT NULL UNIQUE,"
                      "LIST TEXT NOT NULL"
                      ")"};
    char* errMsg{nullptr};
    int rc{sqlite3_exec(db, sql, nullptr, nullptr, &errMsg)};
    if (rc != SQLITE_OK) {
        SPDLOG_ERROR("EIR setup error.");
        sqlite3_free(errMsg);
        return rc;
    }
    return OK_CODE;
}

EIR::EIR(){
    int rc{sqlite3_open("eir.sqlite", &db)};
    if (rc != SQLITE_OK) {
        EIRWorkingState = WorkingState::NOT_WORKING;
        return;
    }
    if (setup() != OK_CODE){
        EIRWorkingState = WorkingState::NOT_WORKING;
        return;
    }
}

EIR::~EIR(){
    sqlite3_close(db);
}

bool EIR::IMEILegal(std::string_view IMEI) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT LIST FROM eir WHERE IMEI = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("EIR prepare failed.");
        return false; 
    }
    
    sqlite3_bind_text(stmt, 1, IMEI.data(), static_cast<int>(IMEI.size()), SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    
    bool legal{false};
    if (rc == SQLITE_ROW) {
        const char* list{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))};
        if (list) {
            std::string status(list);
            if (status == "white") {
                legal = true;     
            } else if (status == "grey") {
                // grey equals to white here
                // but if we want we can change the logic
                legal = true;     
            } else if (status == "black") {
                legal = false;     
            }
        }
    } else {
        legal = true;
    }
    
    sqlite3_finalize(stmt);
    return legal;
}

SUCCESS_RESULT EIR::add(std::string_view IMEI) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"INSERT OR IGNORE INTO eir (IMEI, LIST) VALUES (?, 'white')"};

    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("EIR prepare failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 1, IMEI.data(), static_cast<int>(IMEI.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("EIR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        SPDLOG_WARN("EIR insert failed.");
        sqlite3_finalize(stmt);
        return rc; 
    }

    sqlite3_finalize(stmt);
    return OK_CODE;
}