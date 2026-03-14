#pragma once
#include <vector>
#include <string>
#include "database_handler.hpp"
#include "static_file.hpp"

class StaticFileRepository
{
    private:
        DatabaseHandler* m_pDB;

    public:
        StaticFileRepository(DatabaseHandler* db);

        bool CreateTable();

        bool AddFile(int routeId,
                    const std::string& filePath,
                    const std::string& contentType);

        std::vector<StaticFile> GetFiles();
};

