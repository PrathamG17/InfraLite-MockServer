#pragma once
#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <string>
#include <map>

class HttpRequest
{
private:
    std::string sMethod;                         // GET, POST
    std::string sPath;                           // /index.html
    std::string sHttpVersion;                    // HTTP/1.1
    std::map<std::string, std::string> mHeaders; // key-value pairs of headers
    std::string sBody;                           // request body (for POST/PUT)

public:
    HttpRequest() = default;                //compiler generated default constructor.

    // Static factory method to parse raw HTTP request string
    static HttpRequest Parse(const std::string& rawRequest);

    const std::string& GetPath()const;
    const std::string& GetMethod()const;
    const std::string& GetBody()const;

    std::string GetHeader(const std::string& key) const;
};

#endif // REQUEST_HPP
