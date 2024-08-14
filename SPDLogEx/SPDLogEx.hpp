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
    explicit SpdLogger(const std::string& logFilePath, size_t queueSize = 8192, size_t threadCount = 1);
    void setLevel(LogLevel level) override;
    void setFilePath(const std::string& filePath) override;
    void log(LogLevel level, const std::string& message, const std::string& fileName, int lineNumber) override;

    void trace(const std::string& msg, const char* file = "", int line = 0) override;
    void debug(const std::string& msg, const char* file = "", int line = 0) override;
    void info(const std::string& msg, const char* file = "", int line = 0) override;
    void warn(const std::string& msg, const char* file = "", int line = 0) override;
    void error(const std::string& msg, const char* file = "", int line = 0) override;
    void critical(const std::string& msg, const char* file = "", int line = 0) override;

private:
    void logWithLocation(LogLevel level, const std::string& msg, const char* file, int line);

    template<typename... Args>
    void logFormat(LogLevel level, const char* file, int line, spdlog::format_string_t<Args...> fmt, Args&&... args);

    std::shared_ptr<spdlog::logger> logger;
    std::string filePath;
    spdlog::level::level_enum levelToSpdlogLevel(LogLevel level);
};

};  // namespace Trace
