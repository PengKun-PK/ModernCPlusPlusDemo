#pragma once

#include <map>
#include <string>
#include <fstream>
#include <iostream>
#include <string>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/async.h>
#include <spdlog/async_logger.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/details/thread_pool-inl.h>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

// 默认的日志输出格式
#define LOG_OUTPUT_FORMAT      "%^[ %Y-%m-%d %H:%M:%S.%e ] <thread %t> [%n] [%l]\n%@,%!\n%v%$\n"
// 封装宏
#define LOG_TRACE(loggerName, ...)  SPDLOG_LOGGER_CALL(spdlog::get(loggerName), spdlog::level::trace, __VA_ARGS__)
#define LOG_DEBUG(loggerName, ...)  SPDLOG_LOGGER_CALL(spdlog::get(loggerName), spdlog::level::debug, __VA_ARGS__)
#define LOG_INFO(loggerName, ...)   SPDLOG_LOGGER_CALL(spdlog::get(loggerName), spdlog::level::info, __VA_ARGS__)
#define LOG_WARN(loggerName, ...)   SPDLOG_LOGGER_CALL(spdlog::get(loggerName), spdlog::level::warn, __VA_ARGS__)
#define LOG_ERROR(loggerName, ...)  SPDLOG_LOGGER_CALL(spdlog::get(loggerName), spdlog::level::err, __VA_ARGS__)
#define LOG_CRITI(loggerName, ...)  SPDLOG_LOGGER_CALL(spdlog::get(loggerName), spdlog::level::critical, __VA_ARGS__)

namespace Trace
{

using namespace spdlog;

using namespace std;
class SPDLogEx
{
public:
    SPDLogEx();
    virtual ~SPDLogEx();

    // 日志输出模式
	enum OutMode
    {
		SYNC	= 0,	// 同步模式
		ASYNC	= 1,	// 异步模式
	};
	static OutMode GetOutModeEnum(const std::string& strMode);

	// 日志输出等级
	enum OutLevel
    {
		LEVEL_TRACE = 0,
		LEVEL_DEBUG = 1,
		LEVEL_INFO = 2,
		LEVEL_WARN = 3,
		LEVEL_ERROR = 4,
		LEVEL_CRITI = 5,
		LEVEL_OFF = 6,
	};
	static OutLevel GetOutLevelEnum(const std::string& strLevel);

    // 控制台
	//@pLoggerName	[in]: 记录器名称
	//@eLevel	    [in]: 日志输出等级
	bool AddColorConsole(const char* pLoggerName, const OutLevel level = LEVEL_TRACE);

	// 旋转文件 （到了限定的大小就会重新生成新的文件）
	//@pLoggerName	[in]: 记录器名称
	//@pFileName	[in]: 日志名称
	//@nMaxFileSize	[in]: 生成的文件内容最大容量，单位byte
	//@nMaxFile     [in]: 生成的文件最大数量
	//@eLevel	    [in]: 日志输出等级
	bool AddRotatingFile(const char* pLoggerName, const char* pFileName, const int nMaxFileSize = 1024 * 1024 * 10,
							const int nMaxFile = 10, const OutLevel level = LEVEL_TRACE);

	// 日期文件（在每天的指定时间生成一个日志文件, 文件名以日期命名）
	//@pLoggerName	[in]: 记录器名称
	//@pFileName	[in]: 日志名称
	//@nHour	    [in]: 指定生成时间的时
	//@nMinute      [in]: 指定生成时间的分
	//@eLevel	    [in]: 日志输出等级
	bool AddDailyFile(const char* pLoggerName, const char* pFileName, const int nHour, const int nMinute, const OutLevel eLevel = LEVEL_TRACE);

	// 初始化-软件方式
	bool Init(const OutMode outMode = SYNC, const string strLogFormat = LOG_OUTPUT_FORMAT);
	void UnInit();

private:
    bool m_bInit;
	//<logger名称，logger需要初始化的sinks>：存储初始化前的sink（存在一个logger有多个sink，且有多个logger的情况）
	std::map<std::string, std::vector<spdlog::sink_ptr>> m_mapLoggerParam;
	//<logger名称，logger异步需要的线程池>：由于记录器获取线程池的weak_ptr，所以线程池对象必须比记录器对象的寿命长
	std::map<std::string, std::shared_ptr<spdlog::details::thread_pool>> m_mapAsyncThreadPool;

    std::vector<std::string> StringSplit(const std::string& strSrc, const std::string& strSplit);
	// 更新记录器需要的sink容器
	void UpdateSinkMap(std::string strLoggerName, spdlog::sink_ptr pSink);
	// 更新异步记录器需要的线程池
	void UpdateThreadPoolMap(std::string strLoggerName, std::shared_ptr<spdlog::details::thread_pool> pThreadPool);
};

};