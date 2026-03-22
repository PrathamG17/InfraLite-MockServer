#include "server.hpp"
#include "threadpool.hpp"
#include <iostream>
#include <chrono>

#pragma comment(lib, "Ws2_32.lib")

// Old constructor
CServer::CServer(int iPort,
    Router& rRouter,
    Logger& pLogger,
    AccessLogRepository* logRepo)
    : iPort(iPort),
    rRouter(rRouter),
    iServerFd(-1),
    bIsRunning(false),
    rLogger(pLogger),
    m_pLogRepo(logRepo),
    pSecurity(nullptr),
    pTLS(nullptr)
{
    rLogger.Log("Server object created (non-secure) on port " + std::to_string(iPort), ELogLevel::INFO);
}

// New constructor
CServer::CServer(int iPort,
    Router& rRouter,
    Logger& pLogger,
    AccessLogRepository* logRepo,
    SecurityManager& security,
    TLSContext& tls)
    : iPort(iPort),
    rRouter(rRouter),
    iServerFd(-1),
    bIsRunning(false),
    rLogger(pLogger),
    m_pLogRepo(logRepo),
    pSecurity(&security),
    pTLS(&tls)
{
    rLogger.Log("Server object created (secure) on port " + std::to_string(iPort), ELogLevel::INFO);
}

CServer::~CServer()
{
    if (iServerFd != -1) {
        closesocket(iServerFd);
        rLogger.Log("Server socket closed.", ELogLevel::INFO);
    }
    rLogger.Log("Server object destroyed.", ELogLevel::INFO);
}

void CServer::Run()
{
    InitSocket();
    bIsRunning = true;
    rLogger.Log("Server running on port " + std::to_string(iPort), ELogLevel::INFO);

    ThreadPool pool(std::thread::hardware_concurrency());

    while (bIsRunning) {
        int iClientFd = AcceptClient();
        if (iClientFd == -1) {
            rLogger.Log("Failed to accept client connection.", ELogLevel::WARNING);
            continue;
        }

        pool.enqueue([this, iClientFd]() {
            HandleClient(iClientFd);
            });
    }

    closesocket(iServerFd);
    rLogger.Log("Server stopped.", ELogLevel::INFO);
}

void CServer::Stop()
{
    bIsRunning = false;
    closesocket(iServerFd);
    rLogger.Log("Stop signal received. Server loop will exit.", ELogLevel::INFO);
}

void CServer::InitSocket()
{
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        rLogger.Log("WSAStartup failed with error: " + std::to_string(iResult), ELogLevel::LOG_ERROR);
        throw std::runtime_error("WSAStartup failed");
    }

    iServerFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (iServerFd == INVALID_SOCKET) {
        rLogger.Log("socket() failed with error: " + std::to_string(WSAGetLastError()), ELogLevel::LOG_ERROR);
        WSACleanup();
        throw std::runtime_error("Socket creation failed");
    }

    int iOptVal = 1;
    setsockopt(iServerFd, SOL_SOCKET, SO_REUSEADDR, (const char*)&iOptVal, sizeof(iOptVal));

    sockaddr_in service{};
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(iPort);

    if (bind(iServerFd, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        rLogger.Log("bind() failed", ELogLevel::LOG_ERROR);
        closesocket(iServerFd);
        WSACleanup();
        throw std::runtime_error("bind failed");
    }

    if (listen(iServerFd, SOMAXCONN) == SOCKET_ERROR) {
        rLogger.Log("listen() failed", ELogLevel::LOG_ERROR);
        closesocket(iServerFd);
        WSACleanup();
        throw std::runtime_error("listen failed");
    }
    std::cout << "Server listening on https://localhost:8080" << std::endl;
}

int CServer::AcceptClient() const
{
    return accept(iServerFd, nullptr, nullptr);
}

void CServer::HandleClient(int iClientFd)
{
    if (pTLS) {
        // Secure mode with TLS
        SSL* ssl = SSL_new(pTLS->GetCTX());
        SSL_set_fd(ssl, iClientFd);

        if (SSL_accept(ssl) <= 0) {
            rLogger.Log("TLS handshake failed", ELogLevel::LOG_ERROR);
            SSL_free(ssl);
            closesocket(iClientFd);
            return;
        }

        std::string sRawRequest = ReadRequest(ssl);
        if (sRawRequest.empty()) {
            rLogger.Log("Empty request received.", ELogLevel::WARNING);
            SSL_shutdown(ssl);
            SSL_free(ssl);
            closesocket(iClientFd);
            return;
        }

        HttpRequest rRequest = HttpRequest::Parse(sRawRequest);

        // ? Always use secure routing if SecurityManager is set
        HttpResponse rResponse = pSecurity
            ? rRouter.RouteRequest(rRequest, *pSecurity)
            : rRouter.RouteRequest(rRequest);

        rResponse.mHeaders["Content-Length"] = std::to_string(rResponse.sBody.size());
        if (rResponse.mHeaders.find("Content-Type") == rResponse.mHeaders.end()) {
            rResponse.mHeaders["Content-Type"] = "text/html";
        }

        SendResponse(ssl, rResponse);

        SSL_shutdown(ssl);
        SSL_free(ssl);
        closesocket(iClientFd);
    }
    else {
        // Non-secure mode (plain TCP)
        std::string sRawRequest = ReadRequest(iClientFd);
        if (sRawRequest.empty()) {
            closesocket(iClientFd);
            return;
        }

        HttpRequest rRequest = HttpRequest::Parse(sRawRequest);

        // ? Use secure routing if SecurityManager is set, otherwise fallback
        HttpResponse rResponse = pSecurity
            ? rRouter.RouteRequest(rRequest, *pSecurity)
            : rRouter.RouteRequest(rRequest);

        rResponse.mHeaders["Content-Length"] = std::to_string(rResponse.sBody.size());
        if (rResponse.mHeaders.find("Content-Type") == rResponse.mHeaders.end()) {
            rResponse.mHeaders["Content-Type"] = "text/html";
        }

        SendResponse(iClientFd, rResponse);
        closesocket(iClientFd);
    }
}

std::string CServer::ReadRequest(SSL* ssl) {
    std::string request;
    char buffer[4096];
    int bytesRead = SSL_read(ssl, buffer, sizeof(buffer));
    if (bytesRead <= 0) return "";

    request.append(buffer, bytesRead);

    // Find end of headers
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        HttpRequest temp = HttpRequest::Parse(request.substr(0, headerEnd + 4));
        std::string lenStr = temp.GetHeader("Content-Length");

        if (!lenStr.empty()) {
            int contentLength = std::stoi(lenStr);
            int alreadyHave = static_cast<int>(request.size() - (headerEnd + 4));
            int remaining = contentLength - alreadyHave;

            while (remaining > 0) {
                int n = SSL_read(ssl, buffer, sizeof(buffer));
                if (n <= 0) break;
                request.append(buffer, n);
                remaining -= n;
            }
        }
    }
    return request;
}

void CServer::SendResponse(SSL* ssl, const HttpResponse& rResponse)
{
    std::string sRawResponse = rResponse.ToString();
    SSL_write(ssl, sRawResponse.c_str(), (int)sRawResponse.size());
}

std::string CServer::ReadRequest(int clientFd) {
    std::string request;
    char buffer[4096];
    int bytesRead = recv(clientFd, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) return "";

    request.append(buffer, bytesRead);

    // Find end of headers
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd != std::string::npos) {
        // Parse headers only (up to headerEnd)
        HttpRequest temp = HttpRequest::Parse(request.substr(0, headerEnd + 4));
        std::string lenStr = temp.GetHeader("Content-Length");

        if (!lenStr.empty()) {
            int contentLength = std::stoi(lenStr);
            int alreadyHave = static_cast<int>(request.size() - (headerEnd + 4));
            int remaining = contentLength - alreadyHave;

            while (remaining > 0) {
                int n = recv(clientFd, buffer, sizeof(buffer), 0);
                if (n <= 0) break;
                request.append(buffer, n);
                remaining -= n;
            }
        }
    }
    return request;
}
void CServer::SendResponse(int iClientFd, const HttpResponse& rResponse)
{
    std::string sRawResponse = rResponse.ToString();
    send(iClientFd, sRawResponse.c_str(), (int)sRawResponse.size(), 0);
}

