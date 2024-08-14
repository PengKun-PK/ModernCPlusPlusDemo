#pragma once

#include <chrono>
#include <fstream>
#include <iostream>
#include <map>
#include <string>

namespace Trace
{

// 日志级别枚举
enum class LogLevel
{
    trace,
    debug,
    info,
    warn,
    err,
    critical,
    off
};

// 日志接口类
class ILogger
{
public:
    virtual ~ILogger() = default;

    virtual void setLevel(LogLevel level) = 0;
    virtual void setFilePath(const std::string& filePath) = 0;
    virtual void log(LogLevel level, const std::string& message, const std::string& fileName, int lineNumber) = 0;

    virtual void trace(const std::string& msg, const char* file = "", int line = 0) = 0;
    virtual void debug(const std::string& msg, const char* file = "", int line = 0) = 0;
    virtual void info(const std::string& msg, const char* file = "", int line = 0) = 0;
    virtual void warn(const std::string& msg, const char* file = "", int line = 0) = 0;
    virtual void error(const std::string& msg, const char* file = "", int line = 0) = 0;
    virtual void critical(const std::string& msg, const char* file = "", int line = 0) = 0;

    // 获取日志记录器实例（惰性初始化）
    static std::shared_ptr<ILogger> getLogger(const std::string& logFilePath = "logs/log.txt");

    // 禁用拷贝构造和赋值操作符
    ILogger(const ILogger&) = delete;
    ILogger& operator=(const ILogger&) = delete;

protected:
    // 允许子类构造
    ILogger() = default;
};

class LoggerWrapper
{
public:
    explicit LoggerWrapper(const std::string& filename)
        : m_logger(ILogger::getLogger(filename))
    {
        m_logger->setLevel(LogLevel::debug);
        m_logger->setFilePath("Logs/" + getCurrentDateTime() + ".txt");
    }

    void log(const std::string& message, LogLevel level = LogLevel::info, const char* file = __FILE__,
             int line = __LINE__)
    {
        switch (level)
        {
            case LogLevel::trace:
                m_logger->trace(message, file, line);
                break;
            case LogLevel::debug:
                m_logger->debug(message, file, line);
                break;
            case LogLevel::info:
                m_logger->info(message, file, line);
                break;
            case LogLevel::warn:
                m_logger->warn(message, file, line);
                break;
            case LogLevel::err:
                m_logger->error(message, file, line);
                break;
            case LogLevel::critical:
                m_logger->critical(message, file, line);
                break;
            default:
                m_logger->info(message, file, line);
        }
    }

    // 禁用拷贝构造和赋值操作符
    LoggerWrapper(const LoggerWrapper&) = delete;
    LoggerWrapper& operator=(const LoggerWrapper&) = delete;

    // 允许移动构造和赋值
    LoggerWrapper(LoggerWrapper&&) = default;
    LoggerWrapper& operator=(LoggerWrapper&&) = default;

private:
    std::shared_ptr<ILogger> m_logger;

    static std::string getCurrentDateTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y_%m_%d_%H_%M_%S");
        return ss.str();
    }
};

#define LOG_INFO(logger, message) logger.log(message, LogLevel::info, __FILE__, __LINE__)
#define LOG_DEBUG(logger, message) logger.log(message, LogLevel::debug, __FILE__, __LINE__)
#define LOG_WARN(logger, message) logger.log(message, LogLevel::warn, __FILE__, __LINE__)
#define LOG_ERROR(logger, message) logger.log(message, LogLevel::err, __FILE__, __LINE__)
#define LOG_CRITICAL(logger, message) logger.log(message, LogLevel::critical, __FILE__, __LINE__)

};  // namespace Trace
