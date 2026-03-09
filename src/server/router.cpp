#include "router.hpp"
#include <iostream>

// Constructor: initialize with FileHandler
Router::Router(FileHandler* pHandler): pFileHandler(pHandler) {}

// AddRoute: register a new route manually
void Router::AddRoute(const std::string& sMethod, const std::string& sPath,std::function<HttpResponse(const HttpRequest&)> fnHandler)
{
    std::string sKey = sMethod + ":" + sPath;
    mRoutes[sKey] = fnHandler;
}

// LoadRoutes: integrate with ConfigLoader
void Router::LoadRoutes(const ConfigLoader& rConfig, Logger& rLogger)
{
    try
    {
        const auto& vRoutes = rConfig.GetRoutes();

        for (const auto& rRoute : vRoutes)
        {
            std::string sKey = rRoute.sMethod + ":" + rRoute.sPath;

            auto fnHandler = [rRoute, this](const HttpRequest& request) -> HttpResponse
                {
                    HttpResponse rResponse;

                    // Delay simulation
                    if (rRoute.iDelayMs > 0) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(rRoute.iDelayMs));
                    }

                    // Build response from config
                    rResponse.iStatusCode = rRoute.iStatusCode;          // ? status code
                    rResponse.sStatusText = rRoute.sStatusText;          // ? status text
                    rResponse.eFormat = StringToFormat(rRoute.sResponseType); // ? format
                    rResponse.sBody = rRoute.sResponseBody;              // ? body

                    return rResponse;
                };


            mRoutes[sKey] = fnHandler;
        }
    }
    catch (const std::exception& ex)
    {
        rLogger.Log("Router::LoadRoutes exception -> " + static_cast<std::string>(ex.what()), ELogLevel::LOG_ERROR);
    }
}

// RouteRequest: handle incoming request
HttpResponse Router::RouteRequest(const HttpRequest& request)
{
    HttpResponse rResponse;

    try
    {
        std::string sKey = request.GetMethod() + ":" + request.GetPath();

        auto it = mRoutes.find(sKey);
        if (it != mRoutes.end())
        {
            return it->second(request);
        }

        if (pFileHandler != nullptr)
        {
            return pFileHandler->ServeFile(request.GetPath());
        }

        rResponse.iStatusCode = 404;
        rResponse.sStatusText = "Not Found";
        rResponse.eFormat = EResponseFormat::PLAIN;
        rResponse.sBody = "Route not found: " + request.GetPath();
    }
    catch (const std::exception& ex)
    {
        rResponse.iStatusCode = 500;
        rResponse.sStatusText = "Internal Server Error";
        rResponse.eFormat = EResponseFormat::PLAIN;
        rResponse.sBody = std::string("Router exception: ") + ex.what();
    }

    return rResponse;
}

// Utility function to convert string to EResponseFormat
EResponseFormat Router::StringToFormat(const std::string& sType)
{
    if (sType == "JSON") 
        return EResponseFormat::JSON;
    if (sType == "HTML") 
        return EResponseFormat::HTML;
    if (sType == "XML")  
        return EResponseFormat::XML;
    
    return EResponseFormat::PLAIN;
}
