#pragma once

#include <string>
#include "database_handler.hpp"

class AccessLogRepository
{
    private :
        DatabaseHandler *m_pDB;

    public :
        AccessLogRepository(DatabaseHandler* db);

        bool CreateTable();

        bool AddLog(    const std::string& method,
                        const std::string& path,
                        int statusCode,
                        int responseTime,
                        const std::string& clientIP,
                        const std::string& userAgent);
};
