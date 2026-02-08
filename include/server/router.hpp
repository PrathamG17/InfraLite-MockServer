#pragma once
#ifndef ROUTER_HPP
#define ROUTER_HPP

#include <map>
#include <string>
#include <functional>
#include "logger.hpp"
#include "request.hpp"
#include "response.hpp"
#include "config_loader.hpp"
#include "file_handler.hpp"

class Router
{
private:
    // Map of "METHOD:PATH" -> handler function
    std::map<std::string, std::function<HttpResponse(const HttpRequest&)>> mRoutes;

    FileHandler* pFileHandler;

public:
    Router(FileHandler* pHandler);

    void AddRoute(const std::string& sMethod, const std::string& sPath, std::function<HttpResponse(const HttpRequest&)> fnHandler);

    void LoadRoutes(const ConfigLoader& rConfig, Logger& rLogger);

    HttpResponse RouteRequest(const HttpRequest& request);

    EResponseFormat Router::StringToFormat(const std::string& sType);
};

#endif // ROUTER_HPP
