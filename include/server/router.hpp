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
#include "SecurityManager.hpp"

struct SecureRouteDef {
    std::string method;
    std::string path;
    std::function<HttpResponse(const HttpRequest&)> handler;
    Role requiredRole;
};

class Router
{
private:
    std::map<std::string, std::function<HttpResponse(const HttpRequest&)>> mRoutes;
    std::map<std::string, SecureRouteDef> mSecureRoutes;
    FileHandler* pFileHandler;

public:
    Router(FileHandler* pHandler);

    void AddRoute(const std::string& sMethod, const std::string& sPath,
        std::function<HttpResponse(const HttpRequest&)> fnHandler);

    void AddSecureRoute(const std::string& sMethod, const std::string& sPath,
        std::function<HttpResponse(const HttpRequest&)> fnHandler,
        Role requiredRole);

    void LoadRoutes(const ConfigLoader& rConfig, Logger& rLogger);

    HttpResponse RouteRequest(const HttpRequest& request);
    HttpResponse RouteRequest(const HttpRequest& request, SecurityManager& security);

    EResponseFormat StringToFormat(const std::string& sType);
};

#endif // ROUTER_HPP
