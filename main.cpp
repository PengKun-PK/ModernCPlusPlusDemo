#include <algorithm>
#include <array>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <execution>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <string>

#include "Attribute.hpp"
#include "Camera.hpp"
#include "DataSource.hpp"
#include "Event.hpp"
#include "EventBus.hpp"
#include "ILogger.hpp"
#include "MathFunctions.hpp"
#include "ObjectFactory.hpp"
#include "Observer.hpp"
#include "Singleton.hpp"
#include "StateMachine.hpp"
#include "TemplateClassDemo.hpp"
#include "ThreadPool.hpp"
#include "ThreadPoolInvokeStrategy.hpp"

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
    return static_cast<T>(
        std::min(baseSpeed * std::pow(distance / scaleDistance, exponent), static_cast<double>(maxSpeed)));
}
}  // namespace

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

void testSubscriber(LoggerWrapper& logger)
{
    try
    {
        comm::ThreadPoolInvokeStrategy strategy(8);  // 创建一个有8个线程的策略

        // 事件示例
        comm::Event<int, std::string> myEvent(strategy);
        auto subscription = myEvent.subscribe(
            [&logger](int num, const std::string& str)
            {
                std::ostringstream oss;
                oss << "Event received in thread " << std::this_thread::get_id() << ": " << num << ", " << str;
                logger.log(oss.str());
            });

        // 触发多个事件
        for (int i = 0; i < 10; ++i)
        {
            myEvent.notify(i, "Hello from main thread");
        }

        // 等待一段时间，确保所有事件都被处理
        const auto waitTime = std::chrono::seconds(2);
        std::this_thread::sleep_for(waitTime);

        std::ostringstream oss;
        oss << "Active threads: " << strategy.getActiveThreadCount();
        logger.log(oss.str());

        oss.str("");
        oss << "Queued tasks: " << strategy.getQueueSize();
        logger.log(oss.str());

        strategy.shutdown();  // 显式关闭线程池
    }
    catch (const std::exception& e)
    {
        logger.log("Exception caught: " + std::string(e.what()));
    }
    catch (...)
    {
        logger.log("Unknown exception caught");
    }
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
        logger.log("Task " + std::to_string(i) + " result: " + std::to_string(results[i].get()) + ", Queue size: " +
                   std::to_string(pool.getQueueSize()) + ", Idle threads: " + std::to_string(pool.getIdleThreads()) +
                   ", Active threads: " + std::to_string(pool.getActiveThreads()));
    }

    double avgTime, minTime, maxTime;
    pool.getStats(avgTime, minTime, maxTime);
    std::stringstream ss;
    ss << "Task statistics - Avg time: " << avgTime << "ms, Min time: " << minTime << "ms, Max time: " << maxTime
       << "ms";
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

    std::for_each(std::execution::par_unseq, numArray.begin(), numArray.end(),
                  [&logger](int num) { logger.log("number: " + std::to_string(num), LogLevel::debug); });

    // 测试 calculateSpeed 函数
    double distance = 1000.0;
    logger.log("Speed 1: " + std::to_string(calculateSpeed(distance, SpeedParams1)));
    logger.log("Speed 2: " + std::to_string(calculateSpeed(distance, SpeedParams2)));
    logger.log("Speed 3: " + std::to_string(calculateSpeed(distance, SpeedParams3)));

    // 订阅测试
    testSubscriber(logger);

    return 0;
}
