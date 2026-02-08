#pragma once
#ifndef CONFIG_LOADER_HPP
#define CONFIG_LOADER_HPP

#include <string>
#include <vector>
#include "Logger.hpp"

struct RouteDef
{
    std::string sMethod;       // "GET", "POST"
    std::string sPath;         // "/api/data"
    std::string sResponseType; // "json", "xml", "html", "plain"
    std::string sResponseBody; // body content
};

class ConfigLoader
{
private:
    std::string sConfigPath;        // path to config file
    std::vector<RouteDef> vRoutes;  // loaded routes

public:
    ConfigLoader(const std::string& sPath);

    bool LoadConfig(Logger& rLogger);

    // Access loaded routes
    std::vector<RouteDef> GetRoutes() const;
};

#endif // CONFIG_LOADER_HPP
