#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include "logger.hpp"
#include "router.hpp"
#include "request.hpp"
#include "response.hpp"
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "db/access_log_repository.hpp"

// Forward declaration of Router
class Router;

class CServer
{
private:
    int iPort;                                  // Port number to listen on
    int iServerFd;                              // File descriptor for server socket
    Router& rRouter;                            // Reference to router for handling requests
    bool bIsRunning;                            // Flag to control server loop
    Logger& rLogger;                            //understand all hungarian notations..
    AccessLogRepository* m_pLogRepo;

public:
    CServer(int iPort, Router& rRouter, Logger& rLogger, AccessLogRepository* logRepo);

    ~CServer();

    // Start the server loop
    void Run();

    // Stop the server loop
    void Stop();

private:
    // Initialize socket and bind to port
    void InitSocket();

    // Accept incoming client connection
    int AcceptClient()const;

    // Handle a single client request
    void HandleClient(int iClientFd);

    // Utility: read raw HTTP request from client
    std::string ReadRequest(int iClientFd);

    // Utility: send HTTP response to client
    void SendResponse(int iClientFd, const HttpResponse& rResponse);
};

#endif // SERVER_HPP
