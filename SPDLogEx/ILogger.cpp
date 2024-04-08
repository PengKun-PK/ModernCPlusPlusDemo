#include "ILogger.hpp"
#include "SPDLogEx.hpp"

using namespace Trace;

std::shared_ptr<ILogger> ILogger::getLogger(const std::string& logFilePath) {
    static std::shared_ptr<ILogger> logger = nullptr;
    if (!logger) {
        logger = std::make_shared<SpdLogger>(logFilePath);
    }
    return logger;
}