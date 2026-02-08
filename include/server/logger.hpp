#pragma once
#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>

enum class ELogLevel
{
    INFO,
    WARNING,
    LOG_ERROR
};

class Logger
{
private:
    std::string sLogFilePath;       // Path to log file
    std::ofstream rLogStream;       // File stream for logging

public:
    Logger(const std::string& sPath);

    ~Logger();

    // Log a message with severity level
    void Log(const std::string& sMessage, ELogLevel eLevel);

    bool IsReady() const;
};

#endif // LOGGER_HPP
