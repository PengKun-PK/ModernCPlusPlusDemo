#pragma once

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <string>

namespace Trace
{
using namespace std;

// 日志级别枚举
enum class LogLevel {
    trace,
    debug,
    info,
    warn,
    err,
    critical,
    off
};

// 日志接口类
class ILogger {
public:
    virtual ~ILogger() = default;

    virtual void setLevel(LogLevel level) = 0;
    virtual void setFilePath(const std::string& filePath) = 0;
    virtual void log(LogLevel level, const std::string &message, const std::string &fileName, int lineNumber) = 0;

    virtual void trace(const std::string &msg) = 0;
    virtual void debug(const std::string &msg) = 0;
    virtual void info(const std::string &msg) = 0;
    virtual void warn(const std::string &msg) = 0;
    virtual void error(const std::string &msg) = 0;
    virtual void critical(const std::string &msg) = 0;

    // 获取日志记录器实例（惰性初始化）
    static std::shared_ptr<ILogger> getLogger(const std::string& logFilePath = "logs/log.txt");
};

};