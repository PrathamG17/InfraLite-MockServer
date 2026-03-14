#pragma once
#include <string>

struct StaticFile
{
    int fileId;
    int routeId;
    std::string filePath;
    std::string contentType;
    int isActive;
};

