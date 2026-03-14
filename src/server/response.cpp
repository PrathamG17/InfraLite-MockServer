#include "response.hpp"
#include <sstream>

HttpResponse::HttpResponse(): iStatusCode(200), sStatusText("OK"), mHeaders(), sBody(""), eFormat(EResponseFormat::PLAIN){}

std::string HttpResponse::ToString() const
{
    std::ostringstream rResponseStream;

    rResponseStream << "HTTP/1.1 " << iStatusCode << " " << sStatusText << "\r\n";

    switch (eFormat)
    {
        case EResponseFormat::HTML:
            rResponseStream << "Content-Type: text/html\r\n";
            break;
    
        case EResponseFormat::JSON:
            rResponseStream << "Content-Type: application/json\r\n";
            break;
    
        case EResponseFormat::XML:
            rResponseStream << "Content-Type: application/xml\r\n";
            break;
    
        case EResponseFormat::PLAIN:
        default:
            rResponseStream << "Content-Type: text/plain\r\n";
            break;
    }

    for (const auto& rHeader : mHeaders)
    {
        rResponseStream << rHeader.first << ": " << rHeader.second << "\r\n";
    }

    rResponseStream << "\r\n";

    rResponseStream << sBody;

    return rResponseStream.str();
}

std::string HttpResponse::GetHeader(const std::string& key) const
{
    auto it = mHeaders.find(key);
    if(it != mHeaders.end())
        return it->second;

    return "";
}
