#include "server.hpp"
#include "router.hpp"
#include "request.hpp"
#include "response.hpp"
#include "threadpool.hpp"
#include <iostream>

#include "db/access_log_repository.hpp"

#pragma comment(lib, "Ws2_32.lib")

// Constructor
CServer::CServer(int iPort, Router& rRouter, Logger& pLogger, AccessLogRepository* logRepo) : iPort(iPort), rRouter(rRouter), iServerFd(-1), bIsRunning(false), rLogger(pLogger), m_pLogRepo(logRepo)
{
    rLogger.Log("Server object created on port " + std::to_string(iPort), ELogLevel::INFO);
}

// Destructor
CServer::~CServer()
{
    if (iServerFd != -1)
    {
        closesocket(iServerFd);
        rLogger.Log("Server socket closed.", ELogLevel::INFO);
    }
    rLogger.Log("Server object destroyed.", ELogLevel::INFO);
}

// Start the server loop
void CServer::Run()
{
    // Step 1: Initialize the listening socket
    InitSocket();

    bIsRunning = true;
    rLogger.Log("Server running on port " + std::to_string(iPort), ELogLevel::INFO);

    ThreadPool pool(std::thread::hardware_concurrency()); // e.g. number of cores

    // Step 2: Main accept loop
    while (bIsRunning)
    {
        // Accept a new client connection
        int iClientFd = AcceptClient();
        if (iClientFd == -1)
        {
            rLogger.Log("Failed to accept client connection.", ELogLevel::WARNING);
            continue;
        }

        // Step 3: Handle the client request/response cycle
            //HandleClient(iClientFd);
        // Submit client handling to thread pool
        pool.enqueue([this, iClientFd]() {
            HandleClient(iClientFd);
            });
    }

    // Step 4: Cleanup when loop ends
    closesocket(iServerFd);
    rLogger.Log("Server stopped.", ELogLevel::INFO);
}

// Stop the server loop
void CServer::Stop()
{
    bIsRunning = false;
    closesocket(iServerFd);
    rLogger.Log("Stop signal received. Server loop will exit.", ELogLevel::INFO);
}

// Initialize socket and bind to port
void CServer::InitSocket()
{
    WSADATA wsaData;

    // Initialize Winsock
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        rLogger.Log("WSAStartup failed with error: " + std::to_string(iResult), ELogLevel::LOG_ERROR);
        throw std::runtime_error("WSAStartup failed");
    }

    // Create a socket (IPv4, TCP)
    iServerFd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (iServerFd == INVALID_SOCKET)
    {
        rLogger.Log("socket() failed with error: " + std::to_string(WSAGetLastError()), ELogLevel::LOG_ERROR);
        WSACleanup();
        throw std::runtime_error("Socket creation failed");
    }

    rLogger.Log("Socket created successfully.", ELogLevel::INFO);

    // Step 2: Set socket options
    int iOptVal = 1;
    iResult = setsockopt(
        iServerFd,              // socket descriptor
        SOL_SOCKET,             // level: socket options
        SO_REUSEADDR,           // option: reuse address
        (const char*)&iOptVal,  // pointer to option value
        sizeof(iOptVal)         // size of option value
    );

    if (iResult == SOCKET_ERROR)
    {
        rLogger.Log("setsockopt() failed with error: " + std::to_string(WSAGetLastError()), ELogLevel::LOG_ERROR);
        closesocket(iServerFd);
        WSACleanup();
        throw std::runtime_error("setsockopt failed");
    }

    rLogger.Log("Socket options set successfully (SO_REUSEADDR).", ELogLevel::INFO);

    // Step 3: Bind the socket to the given port
    sockaddr_in service;
    service.sin_family = AF_INET;              // IPv4
    service.sin_addr.s_addr = INADDR_ANY;      // Listen on all available interfaces
    service.sin_port = htons(iPort);           // Convert port to network byte order

    iResult = bind(
        iServerFd,                             // socket descriptor
        (SOCKADDR*)&service,                   // pointer to sockaddr structure
        sizeof(service)                        // size of structure
    );

    if (iResult == SOCKET_ERROR) {
        rLogger.Log("bind() failed with error: " + std::to_string(WSAGetLastError()), ELogLevel::LOG_ERROR);
        closesocket(iServerFd);
        WSACleanup();
        throw std::runtime_error("bind failed");
    }

    rLogger.Log("Socket bound successfully to port " + std::to_string(iPort), ELogLevel::INFO);

    // Step 4: Put the socket into listening mode
    iResult = listen(iServerFd, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        rLogger.Log("listen() failed with error: " + std::to_string(WSAGetLastError()), ELogLevel::LOG_ERROR);
        closesocket(iServerFd);
        WSACleanup();
        throw std::runtime_error("listen failed");
    }

    rLogger.Log("Server is now listening on port " + std::to_string(iPort), ELogLevel::INFO);
}

// Accept incoming client connection
int CServer::AcceptClient()const
{
    sockaddr_in clientInfo;
    int iClientInfoSize = sizeof(clientInfo);

    int iClientFd = accept(
        iServerFd,                          // listening socket
        (SOCKADDR*)&clientInfo,             // client address info
        &iClientInfoSize                    // size of client info
    );

    if (iClientFd == INVALID_SOCKET) {
        rLogger.Log("accept() failed with error: " + std::to_string(WSAGetLastError()), ELogLevel::LOG_ERROR);
        return -1;
    }

    rLogger.Log("Client connected.", ELogLevel::INFO);
    return iClientFd;
}

void CServer::HandleClient(int iClientFd)
{
    try
    {
        // Step 1: Read the raw HTTP request from the client
        std::string sRawRequest = ReadRequest(iClientFd);
        if (sRawRequest.empty())
        {
            rLogger.Log("Empty request received.", ELogLevel::WARNING);
            closesocket(iClientFd);
            return;
        }

        // Step 2: Parse the request into an HttpRequest object
        HttpRequest rRequest = HttpRequest::Parse(sRawRequest);

        // Step 3: Route the request to the appropriate handler
        HttpResponse rResponse = rRouter.RouteRequest(rRequest);

        // Insert entry in ACCESSLOG
        if (m_pLogRepo)
        {
            m_pLogRepo->AddLog(
                rRequest.GetMethod(),
                rRequest.GetPath(),
                rResponse.iStatusCode,
                0,
                "127.0.0.1",
                "Unknown Agent"                
            );
        }

        // Step 4: Ensure mandatory headers (like Content-Length)
        rResponse.mHeaders["Content-Length"] = std::to_string(rResponse.sBody.size());
        if (rResponse.mHeaders.find("Content-Type") == rResponse.mHeaders.end())
        {
            rResponse.mHeaders["Content-Type"] = "text/html"; // default
        }

        // Step 5: Send the response back to the client
        SendResponse(iClientFd, rResponse);

        // Step 6: Close the client socket
        closesocket(iClientFd);
    }
    catch (const std::exception& rEx)
    {
        rLogger.Log("Exception in HandleClient: " + static_cast<std::string>(rEx.what()), ELogLevel::WARNING);

        // Fallback: send 500 Internal Server Error
        HttpResponse rErrorResp;
        rErrorResp.iStatusCode = 500;
        rErrorResp.sStatusText = "Internal Server Error";
        rErrorResp.sBody = "<h1>500 Internal Server Error</h1>";
        rErrorResp.mHeaders["Content-Type"] = "text/html";
        rErrorResp.mHeaders["Content-Length"] = std::to_string(rErrorResp.sBody.size());

        SendResponse(iClientFd, rErrorResp);
        closesocket(iClientFd);
    }
}

// Utility: read raw HTTP request from client
std::string CServer::ReadRequest(int iClientFd)
{
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    std::string request;

    int iResult = recv(iClientFd, buffer, BUFFER_SIZE, 0);
    if (iResult == SOCKET_ERROR) {
        rLogger.Log("recv() failed with error: " + std::to_string(WSAGetLastError()), ELogLevel::LOG_ERROR);
        return "";
    }
    else if (iResult == 0) {
        // Connection closed by client
        rLogger.Log("Client disconnected.", ELogLevel::INFO);
        return "";
    }

    // Append received data to request string
    request.append(buffer, iResult);

    rLogger.Log("Raw HTTP request received:\n" + request, ELogLevel::INFO);
    return request;
}

// Utility: send HTTP response to client
void CServer::SendResponse(int iClientFd, const HttpResponse& rResponse)
{
    // Step 1: Convert HttpResponse to raw HTTP string
    std::string sRawResponse = rResponse.ToString();

    // Step 2: Send the response over the socket
    int iTotalSent = 0;
    int iResponseSize = static_cast<int>(sRawResponse.size());

    while (iTotalSent < iResponseSize)
    {
        int iSent = send(iClientFd, sRawResponse.c_str() + iTotalSent, iResponseSize - iTotalSent, 0);

        if (iSent == SOCKET_ERROR)
        {
            rLogger.Log("Failed to send response to client.", ELogLevel::LOG_ERROR);
            break;
        }

        iTotalSent = iTotalSent + iSent;
    }

    // Step 3: Log for debugging
    rLogger.Log("Sent response (" + std::to_string(iTotalSent) + " bytes)", ELogLevel::INFO);
}
