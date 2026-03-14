#include "db/static_file_repository.hpp"
#include "db/sqlite3.h"

StaticFileRepository::StaticFileRepository(DatabaseHandler* db) : m_pDB(db)
{

}

bool StaticFileRepository::CreateTable()
{
    std::string sql = 
        "CREATE TABLE IF NOT EXISTS STATICFILE ("
        "FileID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "RouteID INTEGER UNIQUE,"
        "FilePath TEXT NOT NULL,"
        "ContentType TEXT,"
        "IsActive INTEGER DEFAULT 1,"
        "FOREIGN KEY(RouteID) REFERENCES MOCKROUTE(RouteID)"
        ");";

    return m_pDB->Execute(sql);
}

bool StaticFileRepository::AddFile(int routeId,
                                    const std::string& filePath,
                                    const std::string& contentType)
{
    const char* sql = 
        "INSERT OR IGNORE INTO STATICFILE (RouteID, FilePath, ContentType) "
        "VALUES (?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB->GetDB(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_int(stmt, 1, routeId);
    sqlite3_bind_text(stmt, 2, filePath.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, contentType.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return success;
}

std::vector<StaticFile> StaticFileRepository::GetFiles()
{
    std::vector<StaticFile> files;

    const char* sql = 
        "SELECT FileID,  RouteID, FilePath, ContentType, IsActive "
        "FROM STATICFILE WHERE IsActive = 1;";

    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB->GetDB(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return files;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        StaticFile f;

        f.fileId = sqlite3_column_int(stmt, 0);
        f.routeId = sqlite3_column_int(stmt, 1);
        f.filePath = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        f.contentType = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        f.isActive = sqlite3_column_int(stmt, 4);

        files.push_back(f);
    }

    sqlite3_finalize(stmt);

    return files;
}

