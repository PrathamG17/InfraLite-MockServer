#pragma once
#include <string>

struct AccessLog
{
    long long logId;
    std::string timestamp;
    std::string method;
    std::string path;
    int statusCode;
    int responseTime;
    std::string clientIP;
    std::string userAgent;
};
