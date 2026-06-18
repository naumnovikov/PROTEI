#include "HLRhandler.h"

#include "spdlog/spdlog.h"

SUCCESS_RESULT HLR::setup() {
    const char* sql{"CREATE TABLE IF NOT EXISTS hlr ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "IMSI TEXT NOT NULL UNIQUE,"
                      "IMEI TEXT NOT NULL UNIQUE,"
                      "MSISDN TEXT NOT NULL UNIQUE,"
                      "MMEID INTEGER NOT NULL,"
                      "STATUS TEXT NOT NULL"
                      ")"};
    char* errMsg{nullptr};
    int rc{sqlite3_exec(db, sql, nullptr, nullptr, &errMsg)};
    if (rc != SQLITE_OK) {
        SPDLOG_ERROR("HLR setup error.");
        sqlite3_free(errMsg);
        return rc;
    }
    return OK_CODE;
}

HLR::HLR(){
    int rc{sqlite3_open("hlr.sqlite", &db)};
    if (rc != SQLITE_OK) {
        HLRWorkingState = WorkingState::NOT_WORKING;
        return;
    }
    if (setup() != OK_CODE){
        HLRWorkingState = WorkingState::NOT_WORKING;
        return;
    }
}

HLR::~HLR(){
    sqlite3_close(db);
}

SUCCESS_RESULT HLR::add(std::string_view IMSI, std::string_view IMEI, std::string_view MSISDN, MME_ID mmeId){
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"INSERT OR IGNORE INTO hlr (IMSI, IMEI, MSISDN, MMEID, STATUS) VALUES (?, ?, ?, ?, 'active')"};

    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR prepare failed.");
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 1, IMSI.data(), static_cast<int>(IMSI.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 2, IMEI.data(), static_cast<int>(IMEI.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 3, MSISDN.data(), static_cast<int>(MSISDN.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_int(stmt, 4, mmeId);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        SPDLOG_WARN("HLR insert failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    if (sqlite3_changes(db) == 0) {
        SPDLOG_WARN("Insert into HLR ignored (Duplicate IMSI or IMEI)");
    }

    sqlite3_finalize(stmt);
    return OK_CODE;    
}

MME_ID HLR::getMMEid_ByMSISDN(std::string_view MSISDN) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT MMEID FROM hlr WHERE MSISDN = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR prepare failed.");
        return MMEID_ERROR; 
    }
    
    rc = sqlite3_bind_text(stmt, 1, MSISDN.data(), static_cast<int>(MSISDN.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR bind failed.");
        sqlite3_finalize(stmt);
        return MMEID_ERROR;
    }
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        MME_ID mmeid {sqlite3_column_int(stmt, 0)};
        sqlite3_finalize(stmt);
        return mmeid;
    } else {
        sqlite3_finalize(stmt);
        return MMEID_ERROR;
    }
}

std::string HLR::getMSISDN_ByIMSI(std::string_view IMSI) {
    std::lock_guard<std::mutex> lock(db_mutex); // Защищаем БД от параллельных запросов
    
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT MSISDN FROM hlr WHERE IMSI = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR prepare failed.");
        return ""; 
    }
    
    rc = sqlite3_bind_text(stmt, 1, IMSI.data(), static_cast<int>(IMSI.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("HLR bind failed.");
        sqlite3_finalize(stmt);
        return "";
    }
    
    rc = sqlite3_step(stmt);
    std::string msisdn{""};
    
    if (rc == SQLITE_ROW) {
        const char* text{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))};
        if (text) {
            msisdn = text;
        }
    }
    
    sqlite3_finalize(stmt);
    return msisdn;
}

bool HLR::hasMSISDN(std::string_view MSISDN) {
    std::lock_guard<std::mutex> lock(db_mutex); 
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT 1 FROM hlr WHERE MSISDN = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_text(stmt, 1, MSISDN.data(), static_cast<int>(MSISDN.size()), SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    
    bool exists{rc == SQLITE_ROW}; 
    sqlite3_finalize(stmt);
    return exists;
}

bool HLR::updateLocation(std::string_view IMSI, MME_ID new_mme_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt = nullptr;
    
    const char* sql{"UPDATE hlr SET MMEID = ? WHERE IMSI = ?"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) return false;
    
    sqlite3_bind_int(stmt, 1, new_mme_id);
    sqlite3_bind_text(stmt, 2, IMSI.data(), static_cast<int>(IMSI.size()), SQLITE_STATIC);
    
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}