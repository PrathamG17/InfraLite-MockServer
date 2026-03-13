#include "db/database_handler.hpp"
#include <iostream>

DatabaseHandler::DatabaseHandler() : m_pDB(nullptr)
{

}

DatabaseHandler::~DatabaseHandler()
{
    Close();
}

bool DatabaseHandler::Open(const std::string& sDBPath)
{
    // code
    int rc = sqlite3_open(sDBPath.c_str(), &m_pDB);

    if(rc)
    {
        std::cerr << "Cannot Open database\n";
        return false;
    }

    return true;
}

void DatabaseHandler::Close()
{
    if (m_pDB)
    {
        sqlite3_close(m_pDB);
        m_pDB = nullptr;
    }
}

bool DatabaseHandler::Execute(const std::string& sSQL)
{
    char* errMsg = nullptr;

    int rc = sqlite3_exec(m_pDB, sSQL.c_str(), nullptr, nullptr, &errMsg);

    if (rc != SQLITE_OK)
    {
        std::cerr << "SQL Error : " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

sqlite3* DatabaseHandler::GetDB()
{
    return m_pDB;
}

