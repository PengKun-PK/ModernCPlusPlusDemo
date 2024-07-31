#include "SPDLogEx.hpp"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <filesystem>

namespace fs = std::filesystem;

using namespace Trace;

SpdLogger::SpdLogger(const std::string& logFilePath)
    : filePath(logFilePath)
{
    fs::path logDir = fs::path(logFilePath).parent_path();

    if (logDir.empty())
    {
        logDir = fs::current_path();
    }

    // Create directory if it doesn't exist
    if (!fs::exists(logDir))
        fs::create_directories(logDir);

    spdlog::init_thread_pool(8192, 1);  // 队列大小8192，线程数1
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::trace);  // 控制台输出级别
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, true);
    file_sink->set_level(spdlog::level::trace);  // 文件输出级别
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
    logger = std::make_shared<spdlog::async_logger>("async_logger", sinks.begin(), sinks.end(), spdlog::thread_pool(),
                                                    spdlog::async_overflow_policy::block);
    logger->set_level(spdlog::level::trace);
    logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    spdlog::register_logger(logger);
}

void SpdLogger::setLevel(LogLevel level)
{
    logger->set_level(levelToSpdlogLevel(level));
}

void SpdLogger::setFilePath(const std::string& filePath)
{
    if (this->filePath != filePath)
    {
        fs::path logDir = fs::path(filePath).parent_path();

        if (logDir.empty())
        {
            logDir = fs::current_path();
        }
        // Create directory if it doesn't exist
        if (!fs::exists(logDir))
            fs::create_directories(logDir);

        this->filePath = filePath;
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
        file_sink->set_level(spdlog::level::trace);

        // Clear existing sinks and add file output sink
        logger->sinks().clear();
        logger->sinks().push_back(file_sink);
        // Add console output sink if not already added
        logger->sinks().push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
    }
}

void SpdLogger::log(LogLevel level, const std::string& message, const std::string& fileName, int lineNumber)
{
    std::stringstream ss;
    ss << message << " [" << fileName << ":" << lineNumber << "]";
    logger->log(levelToSpdlogLevel(level), ss.str());
}

void SpdLogger::trace(const std::string& msg)
{
    log(LogLevel::trace, msg, __FILE__, __LINE__);
}

void SpdLogger::debug(const std::string& msg)
{
    log(LogLevel::debug, msg, __FILE__, __LINE__);
}

void SpdLogger::info(const std::string& msg)
{
    log(LogLevel::info, msg, __FILE__, __LINE__);
}

void SpdLogger::warn(const std::string& msg)
{
    log(LogLevel::warn, msg, __FILE__, __LINE__);
}

void SpdLogger::error(const std::string& msg)
{
    log(LogLevel::err, msg, __FILE__, __LINE__);
}

void SpdLogger::critical(const std::string& msg)
{
    log(LogLevel::critical, msg, __FILE__, __LINE__);
}

spdlog::level::level_enum SpdLogger::levelToSpdlogLevel(LogLevel level)
{
    switch (level)
    {
        case LogLevel::trace:
            return spdlog::level::trace;
        case LogLevel::debug:
            return spdlog::level::debug;
        case LogLevel::info:
            return spdlog::level::info;
        case LogLevel::warn:
            return spdlog::level::warn;
        case LogLevel::err:
            return spdlog::level::err;
        case LogLevel::critical:
            return spdlog::level::critical;
        case LogLevel::off:
            return spdlog::level::off;
        default:
            return spdlog::level::info;
    }
}
