#include <iostream>
#include <csignal>
#include <filesystem>
#include <unordered_map>
#include "server.hpp"
#include "router.hpp"
#include "request.hpp"
#include "response.hpp"
#include "config_loader.hpp"
#include "file_handler.hpp"
#include "logger.hpp"
#include <nlohmann/json.hpp>

#include "db/database_handler.hpp"
#include "db/route_repository.hpp"
#include "db/access_log_repository.hpp"
#include "db/static_file_repository.hpp"
#include "db/user_repository.hpp"
#include "db/server_config_repository.hpp"

// For token generation (dummy example)
#include <string>

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
        if (rLogger.IsReady())
        {
            rLogger.Log("Server starting up...", ELogLevel::INFO);
        }

        ConfigLoader rConfig((baseDir / "config" / "routes.json").string());
        if (!rConfig.LoadConfig(rLogger))
        {
            rLogger.Log("Failed to load routes.json", ELogLevel::LOG_ERROR);
            return 1;
        }

        FileHandler rFileHandler((baseDir / "static").string());

        Router rRouter(&rFileHandler);
        rRouter.LoadRoutes(rConfig, rLogger);

        std::filesystem::path dataDir = baseDir / "data";
        std::filesystem::create_directories(dataDir);

        std::filesystem::path dbPath = dataDir / "infralite.db";
        DatabaseHandler rDB;

        if (!rDB.Open(dbPath.string()))
        {
            std::cout << "Database Open() : Failed!!\n";
            return 1;
        }

        ServerConfigRepository rConfigRepo(&rDB);
        rConfigRepo.CreateTable();
        rConfigRepo.SetConfig("PORT", "8080", "Server listening port");
        rConfigRepo.SetConfig("STATIC_DIR", "static", "Static files directory");
        rConfigRepo.SetConfig("LOG_LEVEL", "INFO", "Logging level");

        UserRepository rUserRepo(&rDB);
        rUserRepo.CreateTable();
        int adminId = rUserRepo.AddUser("admin", "admin12hash", "admin");

        RouteRepository rRepo(&rDB);
        rRepo.CreateTable();

        StaticFileRepository rFileRepo(&rDB);
        rFileRepo.CreateTable();

        rRepo.AddRoute(adminId, "GET", "/dbhello", 200, "");
        rFileRepo.AddFile(1, "index.html", "text/html");
        rRepo.AddRoute(adminId, "GET", "/dbtest", 200, "<h1>Second DB route!!<h1>");

        int bannerID = rRepo.AddRoute(adminId, "GET", "/banner", 200, "");
        rFileRepo.AddFile(bannerID, "index.html", "text/html");

        int logoID = rRepo.AddRoute(adminId, "GET", "/logo", 200, "");
        rFileRepo.AddFile(logoID, "index.html", "text/html");

        auto routes = rRepo.GetRoutes();
        auto files = rFileRepo.GetFiles();
        std::unordered_map<int, StaticFile> fileMap;
        for (const auto& f : files) { fileMap[f.routeId] = f; }

        for (const auto& r : routes)
        {
            auto it = fileMap.find(r.routeId);
            if (it != fileMap.end())
            {
                StaticFile f = it->second;
                rLogger.Log("Static Route Loaded: " + r.method + " " + r.path, ELogLevel::INFO);
                rRouter.AddRoute(r.method, r.path,
                    [&rFileHandler, f](const HttpRequest& req) {
                        return rFileHandler.ServeFile(f.filePath);
                    });
            }
            else
            {
                rLogger.Log("DB Route Loaded: " + r.method + " " + r.path, ELogLevel::INFO);
                rRouter.AddRoute(r.method, r.path,
                    [r](const HttpRequest& req) {
                        HttpResponse resp;
                        resp.iStatusCode = r.responseStatus;
                        resp.sStatusText = "OK";
                        resp.mHeaders["Content-Type"] = "text/html";
                        resp.sBody = r.responseBody;
                        return resp;
                    });
            }
        }

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

        rRouter.AddRoute("POST", "/submit", [](const HttpRequest& rReq) {
            HttpResponse rResp;
            rResp.mHeaders["Content-Type"] = "application/json";

            std::string body = rReq.GetBody();  // or rReq.GetRawBody() if available
            if (body.empty()) {
                rResp.iStatusCode = 400;
                rResp.sStatusText = "Bad Request";
                rResp.sBody = R"({"error":"missing data"})";
                return rResp;
            }

            try {
                auto j = nlohmann::json::parse(body);
                if (!j.contains("data") || j["data"].is_null()) {
                    rResp.iStatusCode = 400;
                    rResp.sStatusText = "Bad Request";
                    rResp.sBody = R"({"error":"missing data"})";
                }
                else {
                    rResp.iStatusCode = 200;
                    rResp.sStatusText = "OK";
                    rResp.sBody = "Data received: " + j["data"].get<std::string>();
                }
            }
            catch (const std::exception& e) {
                rResp.iStatusCode = 400;
                rResp.sStatusText = "Bad Request";
                rResp.sBody = R"({"error":"invalid json"})";
            }

            return rResp;
            });

        // Secure routes using JWT + role enforcement
        rRouter.AddSecureRoute("GET", "/secure/admin",
            [](const HttpRequest& req) {
                HttpResponse resp;
                resp.iStatusCode = 200;
                resp.sStatusText = "OK";
                resp.mHeaders["Content-Type"] = "text/plain";
                resp.sBody = "Welcome, admin! Secure content.";
                return resp;
            },
            Role::Admin   // required role
        );

        rRouter.AddSecureRoute("GET", "/secure/qa",
            [](const HttpRequest& req) {
                HttpResponse resp;
                resp.iStatusCode = 200;
                resp.sStatusText = "OK";
                resp.mHeaders["Content-Type"] = "text/plain";
                resp.sBody = "Hello QA team!";
                return resp;
            },
            Role::QA
        );

        rRouter.AddSecureRoute("GET", "/secure/viewer",
            [](const HttpRequest& req) {
                HttpResponse resp;
                resp.iStatusCode = 200;
                resp.sStatusText = "OK";
                resp.mHeaders["Content-Type"] = "text/plain";
                resp.sBody = "Viewer access granted.";
                return resp;
            },
            Role::Viewer
        );


        AccessLogRepository rLogRepo(&rDB);
        rLogRepo.CreateTable();

        int port = std::stoi(rConfigRepo.GetConfig("PORT"));

        std::string secret = std::getenv("JWT_SECRET") ? std::getenv("JWT_SECRET") : "localdevsecret";
        JWTVerifier verifier(secret, "HS256");

        // Generate a real JWT token string for testing
        std::map<std::string, std::string> claims = {
            {"user", "admin"},
            {"role", "Admin"}
        };
        std::string testToken = verifier.Generate(claims);

        // Log the generated token
        rLogger.Log("Generated test JWT token: " + testToken, ELogLevel::INFO);

        // Pass verifier into your security manager
        SecurityManager security(verifier);

        // Initialize TLS context with certificate and private key files
        auto baseDir = GetProjectRoot();
        TLSContext tlsCtx((baseDir / "certs" / "server.crt").string(),
            (baseDir / "certs" / "server.key").string());

        if (!tlsCtx.Init()) {
            rLogger.Log("Failed to initialize TLS context", ELogLevel::LOG_ERROR);
            return 1;
        }

        // Construct server in secure mode
        CServer rServer(port, rRouter, rLogger, &rLogRepo, security, tlsCtx);

        //CServer rServer(port, rRouter, rLogger, &rLogRepo);
        gServer = &rServer;
        SetConsoleCtrlHandler(ConsoleHandler, TRUE);
        rServer.Run();

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
            gServer->Stop();

        return TRUE;
    }
    return FALSE;
}

std::filesystem::path GetProjectRoot()
{
    auto cwd = std::filesystem::current_path();
    for (int i = 0; i < 4; ++i)
    {
        if (std::filesystem::exists(cwd / "config"))
        {
            return cwd;
        }
        cwd = cwd.parent_path();
    }
    return std::filesystem::current_path();
}
