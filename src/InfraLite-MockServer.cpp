#include <iostream>
#include <csignal>
#include "server.hpp"
#include "router.hpp"
#include "request.hpp"
#include "response.hpp"
#include "config_loader.hpp"
#include "file_handler.hpp"
#include "logger.hpp"

CServer* gServer = nullptr;
Logger* gLogger = nullptr;

BOOL WINAPI ConsoleHandler(DWORD signal);

int main(void)
{
    Logger rLogger("D:\\PRATHAMESH\\PDF\\C++\\BOOST\\InfraLite-MockServer\\logs\\InfraLite-MockServer.log");
    try
    {
        gLogger = &rLogger;
        // Step 1: Initialize Logger
        if (rLogger.IsReady())
        {
            rLogger.Log("Server starting up...", ELogLevel::INFO);
        }

        // Step 2: Initialize ConfigLoader
        ConfigLoader rConfig("D:\\PRATHAMESH\\PDF\\C++\\BOOST\\InfraLite-MockServer\\config\\routes.json");
        if (!rConfig.LoadConfig(rLogger))
        {
            rLogger.Log("Failed to load routes.json", ELogLevel::LOG_ERROR);
            return 1;
        }

        // Step 3: Initialize FileHandler
        FileHandler rFileHandler("D:\\PRATHAMESH\\PDF\\C++\\BOOST\\InfraLite-MockServer\\static");

        // Step 4: Initialize Router
        Router rRouter(&rFileHandler);
        rRouter.LoadRoutes(rConfig, rLogger);

        // Step 2: Register GET routes
        rRouter.AddRoute("GET", "/namaskar", [](const HttpRequest& rReq) {
            HttpResponse rResp;
            rResp.iStatusCode = 200;
            rResp.sStatusText = "OK";
            rResp.mHeaders["Content-Type"] = "text/html";
            rResp.sBody = "<h1>Namaskar!</h1>";
            return rResp;
            });

        rRouter.AddRoute("GET", "/json", [](const HttpRequest& rReq) {
            HttpResponse rResp;
            rResp.iStatusCode = 200;
            rResp.sStatusText = "OK";
            rResp.mHeaders["Content-Type"] = "application/json";
            rResp.sBody = R"({"message":"Hello JSON"})";
            return rResp;
            });

        // Step 3: Register POST route
        rRouter.AddRoute("POST", "/submit", [](const HttpRequest& rReq) {
            HttpResponse rResp;
            rResp.iStatusCode = 200;
            rResp.sStatusText = "OK";
            rResp.mHeaders["Content-Type"] = "text/plain";
            rResp.sBody = "Data received: " + rReq.GetBody();
            return rResp;
            });

        // Step 5: Initialize and run CServer
        CServer rServer(8080, rRouter, rLogger);
        gServer = &rServer;
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);            //for windows only...
        rServer.Run(); // enters accept loop

        // Step 6: Graceful shutdown
        if (rLogger.IsReady())
        {
            rLogger.Log("Server shutting down...", ELogLevel::INFO);
        }
    }
    catch (const std::exception& rEx)
    {
        rLogger.Log("Exception in main: " + static_cast<std::string>(rEx.what()), ELogLevel::LOG_ERROR);
    }

    return 0;
}

BOOL WINAPI ConsoleHandler(DWORD signal) 
{
    if (signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT) 
    {
        if (gLogger != nullptr && gLogger->IsReady()) 
            gLogger->Log("Server shutting down due to console event", ELogLevel::INFO);

        if (gServer != nullptr) 
            gServer->Stop(); // custom method to break accept loop

        return TRUE; // handled
    }
    return FALSE;
}
