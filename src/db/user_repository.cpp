#include "db/user_repository.hpp"
#include "db/sqlite3.h"

UserRepository::UserRepository(DatabaseHandler* db) : m_pDB(db)
{

}

bool UserRepository::CreateTable()
{
    std::string sql = 
        "CREATE TABLE IF NOT EXISTS USER ( "
        "UserID INTEGER PRIMARY KEY AUTOINCREMENT,"
        "UserName TEXT UNIQUE NOT NULL,"
        "PasswordHash TEXT NOT NULL,"
        "Role TEXT,"
        "CreateAt DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "IsActive INTEGER DEFAULT 1"
        ");";

    return m_pDB->Execute(sql);
}

int UserRepository::AddUser(const std::string& username,
                            const std::string& passwordHash,
                            const std::string& role)
{
    const char* sql = 
        "INSERT INTO USER (Username, PasswordHash, Role) VALUES (?, ?, ?);";

    sqlite3_stmt * stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB->GetDB(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return -1;

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, passwordHash.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, role.c_str(), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        sqlite3_finalize(stmt);
        return -1;
    }

    int id = sqlite3_last_insert_rowid(m_pDB->GetDB());

    return id;
}

std::vector<User> UserRepository::GetUsers()
{
    std::vector<User> users;

    const char* sql = 
        "SELECT UserID, Username, PasswordHash, Role, CreatedAt, IsActive "
        "FROM USER WHERE IsActive = 1;";

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(m_pDB->GetDB(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return users;

    while(sqlite3_step(stmt) == SQLITE_ROW)
    {
        User u;

        u.userId = sqlite3_column_int(stmt, 0);
        u.username = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        u.passwordHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        u.createdAt = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        u.isActive = sqlite3_column_int(stmt, 5);

        users.push_back(u);
    }

    sqlite3_finalize(stmt);

    return users;
}

