#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include "spdlog/spdlog.h"

enum class ELogLevel {
    INFO,
    WARNING,
    LOG_ERROR
};

class Logger 
{
private:
    std::string sLogFilePath;

public:
    Logger(const std::string& sPath);
    ~Logger();

    void Log(const std::string& sMessage, ELogLevel eLevel);
    bool IsReady() const;
};

#endif // LOGGER_HPP
