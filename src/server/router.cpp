#include "router.hpp"
#include <iostream>
#include <thread>
#include <chrono>

Router::Router(FileHandler* pHandler) : pFileHandler(pHandler) {}

void Router::AddRoute(const std::string& sMethod, const std::string& sPath,
    std::function<HttpResponse(const HttpRequest&)> fnHandler)
{
    std::string sKey = sMethod + ":" + sPath;
    mRoutes[sKey] = fnHandler;
}

void Router::AddSecureRoute(const std::string& sMethod, const std::string& sPath,
    std::function<HttpResponse(const HttpRequest&)> fnHandler,
    Role requiredRole)
{
    std::string sKey = sMethod + ":" + sPath;
    mSecureRoutes[sKey] = { sMethod, sPath, fnHandler, requiredRole };
}

void Router::LoadRoutes(const ConfigLoader& rConfig, Logger& rLogger)
{
    try {
        const auto& vRoutes = rConfig.GetRoutes();
        for (const auto& rRoute : vRoutes)
        {
            std::string sKey = rRoute.sMethod + ":" + rRoute.sPath;
            auto fnHandler = [rRoute, this](const HttpRequest& request) -> HttpResponse {
                HttpResponse rResponse;
                if (rRoute.iDelayMs > 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(rRoute.iDelayMs));
                }
                rResponse.iStatusCode = rRoute.iStatusCode;
                rResponse.sStatusText = rRoute.sStatusText;
                rResponse.eFormat = StringToFormat(rRoute.sResponseType);
                rResponse.sBody = rRoute.sResponseBody;
                return rResponse;
                };
            mRoutes[sKey] = fnHandler;
        }
    }
    catch (const std::exception& ex) {
        rLogger.Log("Router::LoadRoutes exception -> " + static_cast<std::string>(ex.what()), ELogLevel::LOG_ERROR);
    }
}

HttpResponse Router::RouteRequest(const HttpRequest& request)
{
    std::string sKey = request.GetMethod() + ":" + request.GetPath();
    auto it = mRoutes.find(sKey);
    if (it != mRoutes.end()) {
        return it->second(request);
    }
    if (pFileHandler != nullptr) {
        return pFileHandler->ServeFile(request.GetPath());
    }
    HttpResponse resp;
    resp.iStatusCode = 404;
    resp.sStatusText = "Not Found";
    resp.eFormat = EResponseFormat::PLAIN;
    resp.sBody = "Route not found: " + request.GetPath();
    return resp;
}

HttpResponse Router::RouteRequest(const HttpRequest& request, SecurityManager& security)
{
    std::string sKey = request.GetMethod() + ":" + request.GetPath();
    auto itSecure = mSecureRoutes.find(sKey);
    if (itSecure != mSecureRoutes.end()) {
        if (!security.Authorize(request, itSecure->second.requiredRole)) {
            HttpResponse resp;
            resp.iStatusCode = 403;
            resp.sStatusText = "Forbidden";
            resp.eFormat = EResponseFormat::PLAIN;
            resp.sBody = "Access denied";
            return resp;
        }
        return itSecure->second.handler(request);
    }
    return RouteRequest(request);
}

EResponseFormat Router::StringToFormat(const std::string& sType)
{
    if (sType == "JSON") return EResponseFormat::JSON;
    if (sType == "HTML") return EResponseFormat::HTML;
    if (sType == "XML")  return EResponseFormat::XML;
    return EResponseFormat::PLAIN;
}
