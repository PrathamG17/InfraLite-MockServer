#include "db/access_log_repository.hpp"
#include "db/sqlite3.h"

AccessLogRepository::AccessLogRepository(DatabaseHandler* db) : m_pDB(db)
{

}

bool AccessLogRepository::CreateTable()
{
    std::string sql = 
        "CREATE TABLE IF NOT EXISTS ACCESSLOG ("
        "LogID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "Method TEXT NOT NULL,"
        "Path TEXT NOT NULL,"
        "StatusCode INTEGER,"
        "ResponseTime INTEGER,"
        "ClientIP TEXT,"
        "UserAgent TEXT"
        ");";

    return m_pDB->Execute(sql);
}

bool AccessLogRepository::AddLog(const std::string& method,
                                 const std::string& path,
                                 int statusCode,
                                 int responseTime,
                                 const std::string& clientIP,
                                 const std::string& userAgent)
{
    const char* sql = 
    "INSERT INTO ACCESSLOG "
    "(Method, Path, StatusCode, ResponseTime, ClientIP, UserAgent)"
    "VALUES (?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    if(sqlite3_prepare_v2(m_pDB->GetDB(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, method.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, statusCode);
    sqlite3_bind_int(stmt, 4, responseTime);
    sqlite3_bind_text(stmt, 5, clientIP.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, userAgent.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return success;
}
