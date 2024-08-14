#include "SPDLogEx.hpp"
#include <filesystem>
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace fs = std::filesystem;
using namespace Trace;

SpdLogger::SpdLogger(const std::string& logFilePath, size_t queueSize, size_t threadCount)
    : filePath(logFilePath)
{
    fs::path logDir = fs::path(logFilePath).parent_path();

    if (logDir.empty())
    {
        logDir = fs::current_path();
    }

    try
    {
        if (!fs::exists(logDir))
            fs::create_directories(logDir);

        spdlog::init_thread_pool(queueSize, threadCount);
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::trace);
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logFilePath, true);
        file_sink->set_level(spdlog::level::trace);
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};
        logger = std::make_shared<spdlog::async_logger>("async_logger", sinks.begin(), sinks.end(),
                                                        spdlog::thread_pool(), spdlog::async_overflow_policy::block);
        logger->set_level(spdlog::level::trace);
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
        spdlog::register_logger(logger);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to initialize logger: " << e.what() << std::endl;
        throw;
    }
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
        if (!fs::exists(logDir))
            fs::create_directories(logDir);

        this->filePath = filePath;
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(filePath, true);
        file_sink->set_level(spdlog::level::trace);

        logger->sinks().clear();
        logger->sinks().push_back(file_sink);
        logger->sinks().push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
        logger->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%s:%#] %v");
    }
}

void SpdLogger::log(LogLevel level, const std::string& message, const std::string& fileName, int lineNumber)
{
    logger->log(spdlog::source_loc{fileName.c_str(), lineNumber, SPDLOG_FUNCTION}, levelToSpdlogLevel(level), "{}",
                message);
}

void SpdLogger::trace(const std::string& msg, const char* file, int line)
{
    logger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::trace, "{}", msg);
}

void SpdLogger::debug(const std::string& msg, const char* file, int line)
{
    logger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::debug, "{}", msg);
}

void SpdLogger::info(const std::string& msg, const char* file, int line)
{
    logger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::info, "{}", msg);
}

void SpdLogger::warn(const std::string& msg, const char* file, int line)
{
    logger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::warn, "{}", msg);
}

void SpdLogger::error(const std::string& msg, const char* file, int line)
{
    logger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::err, "{}", msg);
}

void SpdLogger::critical(const std::string& msg, const char* file, int line)
{
    logger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, spdlog::level::critical, "{}", msg);
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

void SpdLogger::logWithLocation(LogLevel level, const std::string& msg, const char* file, int line)
{
    logger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, levelToSpdlogLevel(level), "{}", msg);
}

template<typename... Args>
void SpdLogger::logFormat(LogLevel level, const char* file, int line, spdlog::format_string_t<Args...> fmt,
                          Args&&... args)
{
    logger->log(spdlog::source_loc{file, line, SPDLOG_FUNCTION}, levelToSpdlogLevel(level), fmt,
                std::forward<Args>(args)...);
}
