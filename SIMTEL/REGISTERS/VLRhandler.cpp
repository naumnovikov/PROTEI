
#include "VLRhandler.h"

#include "spdlog/spdlog.h"

SUCCESS_RESULT VLR::setup() {
    const char* sql{"CREATE TABLE IF NOT EXISTS vlr ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                      "TMSI INTEGER NOT NULL UNIQUE,"
                      "IMSI TEXT NOT NULL UNIQUE,"
                      "MSISDN TEXT NOT NULL UNIQUE,"
                      "ENODE_B_ID INTEGER NOT NULL"
                      ")"};
    char* errMsg{nullptr};
    int rc{sqlite3_exec(db, sql, nullptr, nullptr, &errMsg)};
    if (rc != SQLITE_OK) {
        SPDLOG_ERROR("VLR setup error.");
        sqlite3_free(errMsg);
        return rc;
    }
    // When we start server again, VLR needs to be cleared
    const char* clear_sql{"DELETE FROM vlr;"};
    if (sqlite3_exec(db, clear_sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        SPDLOG_WARN("Failed to clear VLR table on startup: {}", errMsg);
        sqlite3_free(errMsg);
    } else {
        SPDLOG_INFO("VLR table successfully cleared on SIMTEL startup.");
    }
    return OK_CODE;
}

VLR::VLR(){
    int rc{sqlite3_open("vlr.sqlite", &db)};
    if (rc != SQLITE_OK) {
        VLRWorkingState = WorkingState::NOT_WORKING;
        return;
    }
    if (setup() != OK_CODE){
        VLRWorkingState = WorkingState::NOT_WORKING;
        return;
    }
}

VLR::~VLR(){
    sqlite3_close(db);
}

bool VLR::removeByTMSI(TMSI tmsi) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt = nullptr;
    const char* sql{"DELETE FROM vlr WHERE TMSI = ?"};
    
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK){
        return false;
    }
    
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(tmsi));
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    
    return rc == SQLITE_DONE;
}

bool VLR::hasIMSI(std::string_view IMSI) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT 1 FROM vlr WHERE IMSI = ? LIMIT 1"};

    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) return false;

    sqlite3_bind_text(stmt, 1, IMSI.data(), static_cast<int>(IMSI.size()), SQLITE_STATIC);
    bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);
    return exists;
}

bool VLR::hasTMSI(TMSI TMSI){
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT 1 FROM vlr WHERE TMSI = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR prepare failed.");
        return false;
    }
    
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(TMSI));
    rc = sqlite3_step(stmt);
    
    bool exists{rc == SQLITE_ROW}; 
    
    sqlite3_finalize(stmt);
    return exists;
}

bool VLR::hasMSISDN(std::string_view MSISDN){
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT 1 FROM vlr WHERE MSISDN = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR prepare failed.");
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, MSISDN.data(), static_cast<int>(MSISDN.size()), SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    
    bool exists{rc == SQLITE_ROW}; 
    
    sqlite3_finalize(stmt);
    return exists;
}

uint32_t VLR::getTMSIByMSISDN(std::string_view MSISDN) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT TMSI FROM vlr WHERE MSISDN = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR prepare failed.");
        return ERROR_CODE;
    }
    
    rc = sqlite3_bind_text(stmt, 1, MSISDN.data(), static_cast<int>(MSISDN.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR bind failed.");
        sqlite3_finalize(stmt);
        return ERROR_CODE;
    }
    
    rc = sqlite3_step(stmt);
    uint32_t tmsi{0};
    if (rc == SQLITE_ROW) {
        tmsi = static_cast<uint32_t>(sqlite3_column_int64(stmt, 0));
    }
    
    sqlite3_finalize(stmt);
    return tmsi;
}

MSISDN VLR::getMSISDNByTMSI(TMSI TMSI) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT MSISDN FROM vlr WHERE TMSI = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR prepare failed.");
        return ERROR_MSISDN; 
    }
    
    rc = sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(TMSI));
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR bind failed.");
        sqlite3_finalize(stmt);
        return ERROR_MSISDN;
    }
    
    rc = sqlite3_step(stmt);
    std::string msisdn;
    if (rc == SQLITE_ROW) {
        const char* text{reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0))};
        if (text) {
            msisdn = text;
        }
    }
    
    sqlite3_finalize(stmt);
    return msisdn;
}

SUCCESS_RESULT VLR::add(TMSI TMSI, std::string_view IMSI, std::string_view MSISDN, ENODE_B_ID eNode_B_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"INSERT OR REPLACE INTO vlr (TMSI, IMSI, MSISDN, ENODE_B_ID) VALUES (?, ?, ?, ?)"};

    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR prepare failed.");
        return rc;
    }

    rc = sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(TMSI));
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 2, IMSI.data(), static_cast<int>(IMSI.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 3, MSISDN.data(), static_cast<int>(MSISDN.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_int(stmt, 4, eNode_B_id);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        SPDLOG_WARN("VLR insert failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    sqlite3_finalize(stmt);
    return OK_CODE; 
}

TMSI_And_ENode_b_id VLR::getTMSI_And_ENode_b_id_ByMSISDN(std::string_view MSISDN) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT TMSI, ENODE_B_ID FROM vlr WHERE MSISDN = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR prepare failed.");
        return ERROR_TUPLE;
    }
    
    rc = sqlite3_bind_text(stmt, 1, MSISDN.data(), static_cast<int>(MSISDN.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("VLR bind failed.");
        sqlite3_finalize(stmt);
        return ERROR_TUPLE;
    }
    
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        TMSI tmsi {static_cast<uint32_t>(sqlite3_column_int64(stmt, 0))};
        ENODE_B_ID enodeb_id{sqlite3_column_int(stmt, 1)};
        sqlite3_finalize(stmt);
        return {tmsi, enodeb_id};
    } else {
        sqlite3_finalize(stmt);
        return ERROR_TUPLE;
    }
}

bool VLR::updateENodeB(TMSI tmsi, ENODE_B_ID new_enodeb_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"UPDATE vlr SET ENODE_B_ID = ? WHERE TMSI = ?"};
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("Prepare updateENodeB failed: {}", sqlite3_errmsg(db));
        return false;
    }
    sqlite3_bind_int(stmt, 1, new_enodeb_id);
    sqlite3_bind_int64(stmt, 2, static_cast<sqlite3_int64>(tmsi));
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        SPDLOG_WARN("Update ENODE_B_ID failed: {}", sqlite3_errmsg(db));
        return false;
    }
    return true;
}

int VLR::getENodeBIdByTMSI(TMSI tmsi) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT ENODE_B_ID FROM vlr WHERE TMSI = ? LIMIT 1"};
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) return -1;
    sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(tmsi));
    rc = sqlite3_step(stmt);
    int enb_id = -1;
    if (rc == SQLITE_ROW) {
        enb_id = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);
    return enb_id;
}