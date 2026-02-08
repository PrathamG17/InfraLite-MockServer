#pragma once
#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

enum class EResponseFormat
{
    HTML,
    JSON,
    XML,
    PLAIN
};

class HttpResponse
{
public:
    int iStatusCode;                             // 200, 404
    std::string sStatusText;                     // "OK", "Not Found"
    std::map<std::string, std::string> mHeaders; // key-value pairs of headers
    std::string sBody;                           // response body
    EResponseFormat eFormat;                     // response format type

public:
    HttpResponse();
    std::string ToString() const;
};

#endif // RESPONSE_HPP
