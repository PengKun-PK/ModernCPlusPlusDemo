#pragma once

#include <memory>
#include <string>
#include "ILogger.hpp"
#include "spdlog/spdlog.h"

namespace Trace
{

class SpdLogger : public ILogger
{
public:
    explicit SpdLogger(const std::string& logFilePath = "logs/log.txt");
    void setLevel(LogLevel level) override;
    void setFilePath(const std::string& filePath) override;
    void log(LogLevel level, const std::string& message, const std::string& fileName, int lineNumber) override;

    void trace(const std::string& msg) override;
    void debug(const std::string& msg) override;
    void info(const std::string& msg) override;
    void warn(const std::string& msg) override;
    void error(const std::string& msg) override;
    void critical(const std::string& msg) override;

private:
    std::shared_ptr<spdlog::logger> logger;
    std::string filePath;
    spdlog::level::level_enum levelToSpdlogLevel(LogLevel level);
};

};  // namespace Trace
