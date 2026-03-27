#include "request.hpp"
#include <sstream>
#include <iostream>

HttpRequest HttpRequest::Parse(const std::string& sRawRequest)
{
    HttpRequest rRequest;
    std::istringstream rStream(sRawRequest);
    std::string sRequestLine;

    // Step 1: Request line
    if (std::getline(rStream, sRequestLine)) {
        std::istringstream rLineStream(sRequestLine);
        rLineStream >> rRequest.sMethod;
        rLineStream >> rRequest.sPath;
        rLineStream >> rRequest.sHttpVersion;
    }

    // Step 2: Headers
    std::string sHeaderLine;
    while (std::getline(rStream, sHeaderLine)) {
        if (sHeaderLine == "\r" || sHeaderLine.empty()) break;
        if (!sHeaderLine.empty() && sHeaderLine.back() == '\r') sHeaderLine.pop_back();

        size_t iDelimiterPos = sHeaderLine.find(":");
        if (iDelimiterPos != std::string::npos) {
            std::string sKey = sHeaderLine.substr(0, iDelimiterPos);
            std::string sValue = sHeaderLine.substr(iDelimiterPos + 1);
            if (!sValue.empty() && sValue.front() == ' ') sValue.erase(0, 1);
            rRequest.mHeaders[sKey] = sValue;
        }
    }

    // Step 3: Body
    auto it = rRequest.mHeaders.find("Content-Length");
    if (it != rRequest.mHeaders.end()) {
        int contentLength = std::stoi(it->second);
        std::string body(contentLength, '\0');
        rStream.read(&body[0], contentLength);
        rRequest.sBody = body;
    }
    else {
        // fallback: read rest of stream
        std::string body;
        std::getline(rStream, body, '\0');
        rRequest.sBody = body;
    }

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
