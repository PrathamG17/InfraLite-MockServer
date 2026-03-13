#include <iostream>
#include "db/route_repository.hpp"
#include "db/sqlite3.h"

RouteRepository::RouteRepository(DatabaseHandler* db) : m_pDB(db)
{

}

bool RouteRepository::CreateTable()
{
    // code
    std::string sql = 
        "CREATE TABLE IF NOT EXISTS MOCKROUTE ("
        "RouteID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "CreatedBy INTEGER,"
        "Method TEXT NOT NULL,"
        "Path TEXT NOT NULL,"
        "ResponseStatus INTEGER,"
        "ResponseBody TEXT,"
        "IsActive INTEGER DEFAULT 1,"
        "CreatedAt DATETIME DEFAULT CURRENT_TIMESTAMP,"
    
        "UNIQUE(Method, Path),"
        "CHECK(Method IN ('GET', 'POST', 'PUT', 'DELETE'))"
        ");";

    return m_pDB->Execute(sql);
}

bool RouteRepository::AddRoute( const std::string& method,
                                const std::string& path,
                                int status,
                                const std::string& body)
{
    // code
    const char* sql = 
    "INSERT INTO MOCKROUTE (Method, Path, ResponseStatus, ResponseBody) VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB->GetDB(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, method.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, status);
    sqlite3_bind_text(stmt, 4, body.c_str(), -1, SQLITE_STATIC);

    bool success = sqlite3_step(stmt) == SQLITE_DONE;

    sqlite3_finalize(stmt);

    return success;
}

std::vector<Route> RouteRepository::GetRoutes()
{
    std::vector<Route> routes;

    const char* sql = "SELECT RouteID, Method, Path, ResponseStatus, ResponseBody, IsActive FROM MOCKROUTE WHERE IsActive=1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB->GetDB(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return routes;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Route r;

        r.routeId = sqlite3_column_int(stmt, 0);
        r.method = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        r.path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        r.responseStatus = sqlite3_column_int(stmt, 3);
        r.responseBody = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        r.isActive = sqlite3_column_int(stmt, 5);

        routes.push_back(r);
    }

    sqlite3_finalize(stmt);

    return routes;
}
