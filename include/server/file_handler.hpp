#pragma once
#ifndef FILE_HANDLER_HPP
#define FILE_HANDLER_HPP

#include <string>
#include "response.hpp"

class FileHandler
{
private:
    std::string sRootDir;   // Root directory for serving static files

public:
    FileHandler(const std::string& sDir);
    HttpResponse ServeFile(const std::string& sPath)const;
};

#endif // FILE_HANDLER_HPP
