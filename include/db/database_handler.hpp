#pragma once

#include <string>
#include "db/sqlite3.h"

class DatabaseHandler
{
    private :
        sqlite3* m_pDB;

    public :
        DatabaseHandler();
        ~DatabaseHandler();

        bool Open(const std::string& sDBPath);
        void Close();

        bool Execute(const std::string& sSQL);

        sqlite3* GetDB();
};