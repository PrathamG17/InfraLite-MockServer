#pragma once

#include <vector>
#include "db/database_handler.hpp"
#include "db/server_config.hpp"

class ServerConfigRepository
{
    private :
        DatabaseHandler* m_pDB;

    public :
        ServerConfigRepository(DatabaseHandler* db);

        bool CreateTable();

        bool SetConfig(const std::string& key,
                       const std::string& value,
                       const std::string& description);

        std::string GetConfig(const std::string& key);

        std::vector<ServerConfig> GetAllConfigs();
};
