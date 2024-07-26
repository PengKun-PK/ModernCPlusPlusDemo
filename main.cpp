#include <iostream>
#include <optional>
#include <string>
#include <chrono>
#include <random>
#include <algorithm>
#include <execution>
#include <iomanip>
#include <sstream>
#include <numeric>
#include <fstream>
#include <future>
#include <array>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "Singleton.hpp"
#include "ObjectFactory.hpp"
#include "EventBus.hpp"
#include "Observer.hpp"
#include "MathFunctions.hpp"
#include "Camera.hpp"
#include "DataSource.hpp"
#include "ILogger.hpp"
#include "StateMachine.hpp"
#include "ThreadPool.hpp"
#include "TemplateClassDemo.hpp"

namespace
{
    using namespace MathFunctions;

    using namespace StateMachine;

    using namespace Trace;

    using namespace std::chrono_literals;

    constexpr char DelimiterStart = '#';
    constexpr char DelimiterMiddle = '|';
    constexpr std::array<double, 4> SpeedParams1 = {111.0945, 30.0, 392.9842, 1.972242};
    constexpr std::array<double, 4> SpeedParams2 = {90.03563, 60.0, 314.6796, 14.54655};
    constexpr std::array<double, 4> SpeedParams3 = {60.0, 20.0, 100.0, 1.584963};

    std::string getCurrentDateTime()
    {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y_%m_%d_%H_%M_%S");
        return ss.str();
    }

    std::optional<std::string> getTestString()
    {
        static const std::string testString = "TEST Console Start!";
        return testString.empty() ? std::nullopt : std::make_optional(testString);
    }

    std::optional<std::pair<std::string, std::string>> extractIdAndMessage(const std::string& command)
    {
        auto firstDelim = command.find(DelimiterMiddle);
        auto lastDelim = command.rfind(DelimiterMiddle);

        if (firstDelim == std::string::npos || firstDelim == lastDelim)
        {
            return std::nullopt;
        }

        std::string id = command.substr(firstDelim + 1, lastDelim - firstDelim - 1);
        std::string message = command.substr(lastDelim + 1);
        return std::make_pair(id, message);
    }

    std::optional<std::string> replaceCommandWithString(std::string& messageText)
    {
        auto start = messageText.find(DelimiterStart);
        auto end = messageText.rfind(DelimiterStart);

        if (start == std::string::npos || end == std::string::npos || start == end)
        {
            return std::nullopt;
        }

        auto extracted = extractIdAndMessage(messageText.substr(start + 1, end - start - 1));

        if (!extracted)
        {
            return std::nullopt;
        }

        messageText.replace(start, end - start + 1, extracted->second);
        return extracted->second;
    }

    int longOperation(int id, std::chrono::milliseconds duration)
    {
        std::this_thread::sleep_for(duration);
        return id * 10;
    }

    template<typename T>
    T calculateSpeed(T distance, const std::array<double, 4>& params)
    {
        auto baseSpeed = params[0];
        auto maxSpeed = params[1];
        auto scaleDistance = params[2];
        auto exponent = params[3];
        return static_cast<T>(std::min(baseSpeed * std::pow(distance / scaleDistance, exponent), static_cast<double>(maxSpeed)));
    }
}

class LoggerWrapper
{
public:

    LoggerWrapper(const std::string& filename)
        : m_logger(ILogger::getLogger(filename))
    {
        m_logger->setLevel(LogLevel::debug);
        m_logger->setFilePath("Logs/" + getCurrentDateTime() + ".txt");
    }

    void log(const std::string& message, LogLevel level = LogLevel::info)
    {
        switch (level)
        {
            case LogLevel::debug:
                m_logger->debug(message);
                break;
            case LogLevel::info:
                m_logger->info(message);
                break;
            case LogLevel::warn:
                m_logger->warn(message);
                break;
            case LogLevel::err:
                m_logger->error(message);
                break;
            default:
                m_logger->info(message);
        }
    }

private:
    std::shared_ptr<ILogger> m_logger;
};

void testLogging(LoggerWrapper& logger)
{
    const auto testString = getTestString();
    logger.log(testString.value_or("no msg!!!"));

    const auto& math = Instance<MathFunction>();
    const auto result = math.calDividedFunction(7, 0);
    if (result)
    {
        logger.log(boost::lexical_cast<std::string>(*result));
    }
    else
    {
        logger.log("Invalid result.", LogLevel::err);
    }
}

void testStateMachine(LoggerWrapper& logger)
{
    auto& dataSource = Instance<DataSource>();
    auto cam = std::make_shared<Camera>(dataSource);
    cam->initiate();
    cam->process_event(EvShutterFull("enter shooting"));
    cam->process_event(EvShutterRelease("enter NoShooting"));
    cam->process_event(EvConfig("enter config"));

    auto& sm = Instance<MyStateMachine<void>>();
    logger.log("Starting state machine");
    sm.start();
    sm.process(StateMachine_<void>::Event1{42});
    sm.process(StateMachine_<void>::Event2{"Hello"});
    sm.process(StateMachine_<void>::Event3{});
    sm.process(StateMachine_<void>::Event2{"Internal"});
}

void testThreadPool(LoggerWrapper& logger)
{
    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(100, 2000);

    for (int i = 0; i < 20; ++i)
    {
        results.emplace_back(pool.enqueue(longOperation, i, std::chrono::milliseconds(dis(gen))));
        logger.log("Task " + std::to_string(i) + " submitted. Queue size: " + std::to_string(pool.getQueueSize()) +
                   ", Idle threads: " + std::to_string(pool.getIdleThreads()) +
                   ", Active threads: " + std::to_string(pool.getActiveThreads()));
    }

    for (size_t i = 0; i < results.size(); ++i)
    {
        logger.log("Task " + std::to_string(i) + " result: " + std::to_string(results[i].get()) +
                   ", Queue size: " + std::to_string(pool.getQueueSize()) +
                   ", Idle threads: " + std::to_string(pool.getIdleThreads()) +
                   ", Active threads: " + std::to_string(pool.getActiveThreads()));
    }

    double avgTime, minTime, maxTime;
    pool.getStats(avgTime, minTime, maxTime);
    std::stringstream ss;
    ss << "Task statistics - Avg time: " << avgTime << "ms, Min time: " << minTime << "ms, Max time: " << maxTime << "ms";
    logger.log(ss.str());
    logger.log("Total tasks processed: " + std::to_string(pool.getTotalTasks()));
}

int main()
{
    LoggerWrapper logger("Test.txt");

    testLogging(logger);
    testStateMachine(logger);
    testThreadPool(logger);

    std::vector<int> numArray(1000);
    std::iota(numArray.begin(), numArray.end(), 0);

    std::for_each(std::execution::par_unseq, numArray.begin(), numArray.end(), [&logger](int num) {
        logger.log("number: " + std::to_string(num), LogLevel::debug);
    });

    // 测试 calculateSpeed 函数
    double distance = 1000.0;
    logger.log("Speed 1: " + std::to_string(calculateSpeed(distance, SpeedParams1)));
    logger.log("Speed 2: " + std::to_string(calculateSpeed(distance, SpeedParams2)));
    logger.log("Speed 3: " + std::to_string(calculateSpeed(distance, SpeedParams3)));

    return 0;
}