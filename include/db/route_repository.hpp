#pragma once

#include <string>
#include <vector>
#include "database_handler.hpp"

struct Route
{
    int routeId;
    int createdBy;

    std::string method;
    std::string path;
    
    int responseStatus;
    std::string responseBody;

    bool isActive;
};

class RouteRepository
{
    private : 
        DatabaseHandler* m_pDB;

    public :
        RouteRepository(DatabaseHandler* db);

        bool CreateTable();

        bool AddRoute(  const std::string& method,
                        const std::string& path,
                        int status,
                        const std::string& body
                    );

        std::vector<Route> GetRoutes();
};
