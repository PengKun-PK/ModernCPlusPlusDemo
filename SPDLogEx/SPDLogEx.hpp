#pragma once

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/logger.h>

namespace Trace
{

class SPDLogEx
{
public:
    SPDLogEx();
    virtual ~SPDLogEx();
};

};