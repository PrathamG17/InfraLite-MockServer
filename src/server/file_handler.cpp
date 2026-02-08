#include "file_handler.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

FileHandler::FileHandler(const std::string& sDir): sRootDir(sDir) {}

HttpResponse FileHandler::ServeFile(const std::string& sPath)const
{
    HttpResponse rResponse;

    try
    {
        std::string sFullPath = sRootDir;
        char cSep = '\\';                   // Windows path separator

        if (!sFullPath.empty() && sFullPath.back() != cSep)
        {
            sFullPath = sFullPath + cSep;
        }

        std::string sCleanPath = sPath;
        if (!sCleanPath.empty() && (sCleanPath.front() == '/' || sCleanPath.front() == '\\'))   //condition also contain linux path separator check.
        {
            sCleanPath.erase(0, 1);
        }

        sFullPath = sFullPath + sCleanPath;

        std::ifstream rFileStream(sFullPath, std::ios::binary);
        if (!rFileStream.is_open())
        {
            rResponse.iStatusCode = 404;
            rResponse.sStatusText = "Not Found";
            rResponse.eFormat = EResponseFormat::PLAIN;
            rResponse.sBody = "File not found: " + sFullPath;
            return rResponse;
        }

        std::ostringstream rBuffer;
        rBuffer << rFileStream.rdbuf();
        rResponse.sBody = rBuffer.str();

        if (sFullPath.size() >= 5 && sFullPath.substr(sFullPath.size() - 5) == ".html")
            rResponse.eFormat = EResponseFormat::HTML;
        else if (sFullPath.size() >= 5 && sFullPath.substr(sFullPath.size() - 5) == ".json")
            rResponse.eFormat = EResponseFormat::JSON;
        else if (sFullPath.size() >= 4 && sFullPath.substr(sFullPath.size() - 4) == ".xml")
            rResponse.eFormat = EResponseFormat::XML;
        else
            rResponse.eFormat = EResponseFormat::PLAIN;

        rResponse.iStatusCode = 200;
        rResponse.sStatusText = "OK";
        rResponse.mHeaders["Content-Length"] = std::to_string(rResponse.sBody.size());
    }
    catch (const std::exception& ex)
    {
        rResponse.iStatusCode = 500;
        rResponse.sStatusText = "Internal Server Error";
        rResponse.eFormat = EResponseFormat::PLAIN;
        rResponse.sBody = std::string("FileHandler exception: ") + ex.what();
    }

    return rResponse;
}
