#include "logger.hpp"
#include <sstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <iomanip>

Logger::Logger(const std::string& sPath): sLogFilePath(sPath)
{
    try
    {
        rLogStream.open(sLogFilePath, std::ios::app);
        if (!rLogStream.is_open())
        {
            std::cerr << "Logger: Failed to open log file " << sLogFilePath << std::endl;
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Logger: Exception opening log file -> " << ex.what() << std::endl;
    }
}

Logger::~Logger()
{
    if (rLogStream.is_open())
    {
        rLogStream.close();
    }
}

void Logger::Log(const std::string& sMessage, ELogLevel eLevel)
{
    try
    {
        if (!IsReady())
        {
            std::cerr << "Logger: Not ready, message skipped -> " << sMessage << std::endl;
            return;
        }

        // Get current timestamp
        auto tNow = std::chrono::system_clock::now();
        std::time_t tTime = std::chrono::system_clock::to_time_t(tNow);
        std::tm tmLocal{};

        // Windows safe version only
        localtime_s(&tmLocal, &tTime);

        // Format timestamp
        std::ostringstream rTimeStream;
        rTimeStream << std::put_time(&tmLocal, "%Y-%m-%d %H:%M:%S");

        // Convert log level to string
        std::string sLevel;
        switch (eLevel)
        {
            case ELogLevel::INFO:    
                sLevel = "INFO"; 
                break;

            case ELogLevel::WARNING: 
                sLevel = "WARNING"; 
                break;

            case ELogLevel::LOG_ERROR:   
                sLevel = "ERROR"; 
                break;
        }

        // Write formatted log entry
        rLogStream << "[" << rTimeStream.str() << "] " << "[" << sLevel << "] " << sMessage << std::endl;

        // Flush immediately
        rLogStream.flush();
    }
    catch (const std::exception& ex)
    {
        Log("Logger: Exception during Log -> " + static_cast<std::string>(ex.what()), ELogLevel::LOG_ERROR);
    }
}

bool Logger::IsReady() const
{
    return rLogStream.is_open() && rLogStream.good();
}
