#include "smsc_db.h"

#include <spdlog/spdlog.h>

SMSC_db::SMSC_db(){
    int rc{sqlite3_open("smsc_db.sqlite", &db)};
    if (rc != SQLITE_OK) {
        SMSC_db_WorkingState = WorkingState::NOT_WORKING;
        return;
    }
    if (setup() != OK_CODE){ 
        SMSC_db_WorkingState = WorkingState::NOT_WORKING;
        return;
    }
}

SMSC_db::~SMSC_db(){
    sqlite3_close(db);
}

SUCCESS_RESULT SMSC_db::setup(){ 
    const char* sql{"CREATE TABLE IF NOT EXISTS smsc_db ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "SMS_ID INTEGER NOT NULL UNIQUE,"
                    "SMS_TEXT TEXT NOT NULL,"
                    "TMSI_SRC INTEGER NOT NULL,"
                    "MSISDN_DST TEXT NOT NULL,"
                    "TIMESTAMP TEXT NOT NULL"
                    ")"};
    char* errMsg{nullptr};
    int rc{sqlite3_exec(db, sql, nullptr, nullptr, &errMsg)};
    if (rc != SQLITE_OK) {
        SPDLOG_ERROR("SMSC_DB setup error.");
        sqlite3_free(errMsg);
        return rc;
    }
    return OK_CODE;
}

bool SMSC_db::hasSMSId(SMS_ID sms_id){
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"SELECT 1 FROM smsc_db WHERE SMS_ID = ? LIMIT 1"};
    
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("SMSC_DB prepare failed.");
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, sms_id);
    rc = sqlite3_step(stmt);
    
    bool exists{rc == SQLITE_ROW}; 
    
    sqlite3_finalize(stmt);
    return exists;
}

SUCCESS_RESULT SMSC_db::add(TMSI TMSI_src, std::string_view MSISDN_dst, std::string_view SMS_Text, SMS_ID sms_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};

    const char* sql{"INSERT INTO smsc_db (SMS_ID, SMS_TEXT, TMSI_SRC, MSISDN_DST, TIMESTAMP) VALUES (?, ?, ?, ?, date('now'))"};

    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("SMSC_DB prepare failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_int(stmt, 1, sms_id);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("SMSC_DB bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 2, SMS_Text.data(), static_cast<int>(SMS_Text.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("SMSC_DB bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_int(stmt, 3, TMSI_src);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("SMSC_DB bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_bind_text(stmt, 4, MSISDN_dst.data(), static_cast<int>(MSISDN_dst.size()), SQLITE_STATIC);
    if (rc != SQLITE_OK) {
        SPDLOG_WARN("SMSC_DB bind failed.");
        sqlite3_finalize(stmt);
        return rc;
    }

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        SPDLOG_WARN("SMSC_DB insert failed.");
        sqlite3_finalize(stmt);
        return rc; 
    }

    sqlite3_finalize(stmt);
    return OK_CODE;
}

uint32_t SMSC_db::getSenderTMSI(int sms_id) {
    std::lock_guard<std::mutex> lock(db_mutex); 
    sqlite3_stmt* stmt{nullptr};
    
    const char* sql{"SELECT TMSI_SRC FROM smsc_db WHERE SMS_ID = ? LIMIT 1"};

    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_ERROR("Failed to prepare getSenderTMSI: {}", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_int(stmt, 1, sms_id);

    TMSI sender_tmsi{0};
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        sender_tmsi = static_cast<TMSI>(sqlite3_column_int64(stmt, 0));
    } else {
        SPDLOG_WARN("SMS_ID {} not found in SMSC DB", sms_id);
    }

    sqlite3_finalize(stmt);
    return sender_tmsi;
}

SUCCESS_RESULT SMSC_db::remove(SMS_ID sms_id) {
    std::lock_guard<std::mutex> lock(db_mutex);
    sqlite3_stmt* stmt{nullptr};
    const char* sql{"DELETE FROM smsc_db WHERE SMS_ID = ?"};
    int rc{sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr)};
    if (rc != SQLITE_OK) {
        SPDLOG_ERROR("Prepare delete failed: {}", sqlite3_errmsg(db));
        return rc;
    }
    sqlite3_bind_int(stmt, 1, sms_id);
    rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    if (rc != SQLITE_DONE) {
        SPDLOG_ERROR("Delete failed: {}", sqlite3_errmsg(db));
        return rc;
    }
    SPDLOG_INFO("SMS {} removed from SMSC DB", sms_id);
    return OK_CODE;
}