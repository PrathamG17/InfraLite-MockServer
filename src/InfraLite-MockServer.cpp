#include <iostream>
#include <csignal>
#include <filesystem>
#include "server.hpp"
#include "router.hpp"
#include "request.hpp"
#include "response.hpp"
#include "config_loader.hpp"
#include "file_handler.hpp"
#include "logger.hpp"

#include "db/database_handler.hpp"
#include "db/route_repository.hpp"
#include "db/access_log_repository.hpp"

CServer* gServer = nullptr;
Logger* gLogger = nullptr;

std::filesystem::path GetProjectRoot();
BOOL WINAPI ConsoleHandler(DWORD signal);

int main(void)
{
    auto baseDir = GetProjectRoot();

    Logger rLogger((baseDir / "logs" / "InfraLite-MockServer.log").string());
    try
    {
        gLogger = &rLogger;
        // Step 1: Initialize Logger
        if (rLogger.IsReady())
        {
            rLogger.Log("Server starting up...", ELogLevel::INFO);
        }

        // Step 2: Initialize ConfigLoader
        ConfigLoader rConfig((baseDir / "config" / "routes.json").string());
        if (!rConfig.LoadConfig(rLogger))
        {
            rLogger.Log("Failed to load routes.json", ELogLevel::LOG_ERROR);
            return 1;
        }

        // Step 3: Initialize FileHandler
        FileHandler rFileHandler((baseDir / "static").string());

        // Step 4: Initialize Router
        Router rRouter(&rFileHandler);
        rRouter.LoadRoutes(rConfig, rLogger);


        // initialize Database Handler
        std::filesystem::path dataDir = baseDir / "data";
        std::filesystem::create_directories(dataDir);

        std::filesystem::path dbPath = dataDir / "infralite.db";

        DatabaseHandler rDB;

        if(!rDB.Open(dbPath.string()))
        {
            std::cout << "Database Open() : Failed!!\n";
            return 1;
        }

        RouteRepository rRepo(&rDB);

        // create table if not exists
        rRepo.CreateTable();

        // Inssert test route
        rRepo.AddRoute("GET", "/dbhello", 200, "Hello From SQLite");
        rRepo.AddRoute("GET", "/dbtest", 200, "<h1>Second DB route!!<h1>");

        // Read routes and print to console
        auto routes = rRepo.GetRoutes();

        for(const auto& r : routes)
        {
            rLogger.Log("DB Route Loaded: " + r.method + " " + r.path, ELogLevel::INFO);

            rRouter.AddRoute(r.method, r.path,
                [r](const HttpRequest& req)
            {
                HttpResponse resp;

                resp.iStatusCode = r.responseStatus;
                resp.sStatusText = "OK";
                resp.mHeaders["Content-Type"] = "text/html";
                resp.sBody = r.responseBody;

                return resp;
            });
        }

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
            rResp.mHeaders["Content-Type"] = "text/plain";

            // Simple validation: require non-empty body
            if (rReq.GetBody().empty() || rReq.GetBody() == "{}") {
                rResp.iStatusCode = 400;
                rResp.sStatusText = "Bad Request";
                rResp.sBody = "error: missing data";
            }
            else {
                rResp.iStatusCode = 200;
                rResp.sStatusText = "OK";
                rResp.sBody = "Data received: " + rReq.GetBody();
            }

            return rResp;
            });

        // Create Access Log Repository table
        AccessLogRepository rLogRepo(&rDB);
        rLogRepo.CreateTable();

        rLogRepo.AddLog(
            "GET",
            "/test",
            200,
            5,
            "127.0.0.1",
            "TestAgent"
        );

        // Step 5: Initialize and run CServer
        CServer rServer(8080, rRouter, rLogger, &rLogRepo);
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

std::filesystem::path GetProjectRoot() 
{
    auto cwd = std::filesystem::current_path();

    // Walk up until we find "config"
    for (int i = 0; i < 4; ++i) 
    { // try up to 4 levels
        if (std::filesystem::exists(cwd / "config")) 
        {
            return cwd;
        }
        cwd = cwd.parent_path();
    }

    return std::filesystem::current_path(); // fallback
}
