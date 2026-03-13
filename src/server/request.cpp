#include "request.hpp"
#include <sstream>
#include <iostream>

HttpRequest HttpRequest::Parse(const std::string& sRawRequest)
{
    HttpRequest rRequest;

    std::istringstream rStream(sRawRequest);
    std::string sRequestLine;

    // Step 1: Read the first line (request line)
    if (std::getline(rStream, sRequestLine))
    {
        std::istringstream rLineStream(sRequestLine);

        // Extract method, path, and HTTP version
        rLineStream >> rRequest.sMethod;       // e.g., "GET"
        rLineStream >> rRequest.sPath;         // e.g., "/index.html"
        rLineStream >> rRequest.sHttpVersion;  // e.g., "HTTP/1.1"
    }

    // Step 2: Headers
    std::string sHeaderLine;
    while (std::getline(rStream, sHeaderLine))
    {
        if (sHeaderLine == "\r" || sHeaderLine.empty())
        {
            // Blank line -> end of headers
            break;
        }

        // Remove trailing '\r' if present (Windows CRLF)
        if (!sHeaderLine.empty() && sHeaderLine.back() == '\r')
        {
            sHeaderLine.pop_back();
        }

        // Split header into key and value
        size_t iDelimiterPos = sHeaderLine.find(":");
        if (iDelimiterPos != std::string::npos)
        {
            std::string sKey = sHeaderLine.substr(0, iDelimiterPos);
            std::string sValue = sHeaderLine.substr(iDelimiterPos + 1);

            // Trim leading spaces from value
            if (!sValue.empty() && sValue.front() == ' ')
            {
                sValue.erase(0, 1);
            }

            rRequest.mHeaders[sKey] = sValue;
        }
    }

    // Step 3: Body (everything after headers)
    std::string sBodyContent;
    std::string sLine;
    while (std::getline(rStream, sLine))
    {
        // Remove trailing '\r' if present
        if (!sLine.empty() && sLine.back() == '\r')
        {
            sLine.pop_back();
        }
        sBodyContent += sLine + "\n"; // preserve line breaks
    }

    // Trim final newline if present
    if (!sBodyContent.empty() && sBodyContent.back() == '\n')
    {
        sBodyContent.pop_back();
    }
    rRequest.sBody = sBodyContent;

    return rRequest;
}

const std::string& HttpRequest::GetPath()const
{
    return sPath;
}

const std::string& HttpRequest::GetMethod()const
{
    return sMethod;
}

const std::string& HttpRequest::GetBody()const
{
    return sBody;
}

std::string HttpRequest::GetHeader(const std::string& key) const
{
    auto it = mHeaders.find(key);
    if(it != mHeaders.end())
        return it->second;

    return "";
}
