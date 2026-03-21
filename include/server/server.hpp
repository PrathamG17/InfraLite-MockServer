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
#include "SecurityManager.hpp"
#include "TLSContext.hpp"

class Router;

class CServer
{
private:
    int iPort;
    int iServerFd;
    Router& rRouter;
    bool bIsRunning;
    Logger& rLogger;
    AccessLogRepository* m_pLogRepo;

    // Optional security/TLS
    SecurityManager* pSecurity;
    TLSContext* pTLS;

public:
    // Old constructor (non-secure mode)
    CServer(int iPort,
        Router& rRouter,
        Logger& rLogger,
        AccessLogRepository* logRepo);

    // New constructor (secure mode)
    CServer(int iPort,
        Router& rRouter,
        Logger& rLogger,
        AccessLogRepository* logRepo,
        SecurityManager& security,
        TLSContext& tls);

    ~CServer();

    void Run();
    void Stop();

private:
    void InitSocket();
    int AcceptClient() const;
    void HandleClient(int iClientFd);

    std::string ReadRequest(SSL* ssl);
    void SendResponse(SSL* ssl, const HttpResponse& rResponse);

    // Non-secure helpers
    std::string ReadRequest(int iClientFd);
    void SendResponse(int iClientFd, const HttpResponse& rResponse);
};

#endif // SERVER_HPP
