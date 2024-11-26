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
#include "ObjectFactory.hpp"
#include "Observer.hpp"
#include "OpenCLWrapper.hpp"
#include "Singleton.hpp"
#include "StateMachine.hpp"
#include "TemplateClassDemo.hpp"
#include "ThreadPool.hpp"
#include "ThreadPoolInvokeStrategy.hpp"

namespace
{
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

void testStateMachine(LoggerWrapper& logger)
{
    auto& dataSource = Instance<DataSource>();
    auto cam = std::make_shared<Camera>(dataSource);
    cam->initiate();
    cam->process_event(EvShutterFull("enter shooting"));
    cam->process_event(EvShutterRelease("enter NoShooting"));
    cam->process_event(EvConfig("enter config"));

    auto& sm = Instance<MyStateMachine<void>>();
    LOG_INFO(logger, "Starting state machine");
    sm.start();
    sm.process(StateMachine_<void>::Event1{42});
    sm.process(StateMachine_<void>::Event2{"Hello"});
    sm.process(StateMachine_<void>::Event3{});
    sm.process(StateMachine_<void>::Event2{"Internal"});
}

void complexTestSubscriberSystem(LoggerWrapper& logger)
{
    try
    {
        LOG_INFO(logger, "Starting complex subscriber system test");

        // 创建线程池和调用策略
        comm::ThreadPoolInvokeStrategy strategy(16);  // 使用16个线程

        // 创建多个事件和属性
        comm::Event<int, std::string> intStringEvent(strategy);
        comm::EventSync<double> doubleSyncEvent;
        comm::Event<std::vector<int>> vectorEvent(strategy);
        comm::Attribute<int> intAttribute(strategy, 0);
        comm::AttributeSync<std::string> stringSyncAttribute("");
        comm::Attribute<std::vector<double>> vectorAttribute(strategy, {});

        std::atomic<int> totalNotifications(0);
        std::atomic<int> totalAttributeUpdates(0);
        std::atomic<int> exceptionCount(0);

        // 创建随机数生成器
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> intDis(1, 1000);
        std::uniform_real_distribution<> doubleDis(0.0, 100.0);

        // 创建多个任务
        std::vector<std::future<void>> tasks;
        for (int i = 0; i < 50; ++i)
        {
            tasks.emplace_back(std::async(
                std::launch::async,
                [&, i]()
                {
                    try
                    {
                        // 为每种类型的事件和属性创建订阅存储
                        typename comm::Event<int, std::string>::SubscriptionPtr intStringSubscription;
                        typename comm::EventSync<double>::SubscriptionPtr doubleSyncSubscription;
                        typename comm::Event<std::vector<int>>::SubscriptionPtr vectorSubscription;
                        typename comm::Attribute<int>::SubscriptionPtr intAttributeSubscription;
                        typename comm::AttributeSync<std::string>::SubscriptionPtr stringSyncAttributeSubscription;
                        typename comm::Attribute<std::vector<double>>::SubscriptionPtr vectorAttributeSubscription;

                        // 随机订阅事件和属性
                        if (i % 2 == 0)
                        {
                            intStringSubscription = intStringEvent.subscribe(
                                [&](int n, const std::string& s)
                                {
                                    LOG_INFO(logger, "Task " + std::to_string(i) +
                                                         " received intStringEvent: " + std::to_string(n) + ", " + s);
                                    totalNotifications++;
                                });
                        }
                        if (i % 3 == 0)
                        {
                            doubleSyncSubscription = doubleSyncEvent.subscribe(
                                [&](double d)
                                {
                                    LOG_INFO(logger, "Task " + std::to_string(i) +
                                                         " received doubleSyncEvent: " + std::to_string(d));
                                    totalNotifications++;
                                });
                        }
                        if (i % 4 == 0)
                        {
                            vectorSubscription = vectorEvent.subscribe(
                                [&](const std::vector<int>& v)
                                {
                                    LOG_INFO(logger, "Task " + std::to_string(i) + " received vectorEvent with " +
                                                         std::to_string(v.size()) + " elements");
                                    totalNotifications++;
                                });
                        }
                        if (i % 2 == 1)
                        {
                            intAttributeSubscription = intAttribute.subscribe(
                                [&](int n)
                                {
                                    LOG_INFO(logger, "Task " + std::to_string(i) +
                                                         " received intAttribute update: " + std::to_string(n));
                                    totalAttributeUpdates++;
                                });
                        }
                        if (i % 3 == 1)
                        {
                            stringSyncAttributeSubscription = stringSyncAttribute.subscribe(
                                [&](const std::string& s)
                                {
                                    LOG_INFO(logger, "Task " + std::to_string(i) +
                                                         " received stringSyncAttribute update: " + s);
                                    totalAttributeUpdates++;
                                });
                        }
                        if (i % 4 == 1)
                        {
                            vectorAttributeSubscription = vectorAttribute.subscribe(
                                [&](const std::vector<double>& v)
                                {
                                    LOG_INFO(logger, "Task " + std::to_string(i) +
                                                         " received vectorAttribute update with " +
                                                         std::to_string(v.size()) + " elements");
                                    totalAttributeUpdates++;
                                });
                        }

                        // 执行一些操作
                        for (int j = 0; j < 100; ++j)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(intDis(gen)));

                            if (j % 10 == 0)
                            {
                                intStringEvent.notify(intDis(gen), "Message from task " + std::to_string(i));
                            }
                            if (j % 15 == 0)
                            {
                                doubleSyncEvent.notify(doubleDis(gen));
                            }
                            if (j % 20 == 0)
                            {
                                vectorEvent.notify(std::vector<int>{intDis(gen), intDis(gen), intDis(gen)});
                            }
                            if (j % 12 == 0)
                            {
                                intAttribute.setValue(intDis(gen));
                            }
                            if (j % 18 == 0)
                            {
                                stringSyncAttribute.setValue("Update from task " + std::to_string(i));
                            }
                            if (j % 25 == 0)
                            {
                                vectorAttribute.setValue(
                                    std::vector<double>{doubleDis(gen), doubleDis(gen), doubleDis(gen)});
                            }

                            // 模拟随机异常
                            // if (intDis(gen) % 500 == 0)
                            // {
                            //     throw std::runtime_error("Random exception in task " + std::to_string(i));
                            // }
                        }

                        // 手动取消所有订阅
                        intStringSubscription.reset();
                        doubleSyncSubscription.reset();
                        vectorSubscription.reset();
                        intAttributeSubscription.reset();
                        stringSyncAttributeSubscription.reset();
                        vectorAttributeSubscription.reset();
                    }
                    catch (const std::exception& e)
                    {
                        LOG_ERROR(logger, "Exception in task " + std::to_string(i) + ": " + e.what());
                        exceptionCount++;
                    }
                }));
        }

        // 等待所有任务完成
        for (auto& task : tasks)
        {
            task.wait();
        }

        // 输出统计信息
        LOG_INFO(logger, "Test completed. Statistics:");
        LOG_INFO(logger, "Total notifications: " + std::to_string(totalNotifications));
        LOG_INFO(logger, "Total attribute updates: " + std::to_string(totalAttributeUpdates));
        LOG_INFO(logger, "Total exceptions: " + std::to_string(exceptionCount));
        LOG_INFO(logger, "Final intAttribute value: " + std::to_string(intAttribute.value()));
        LOG_INFO(logger, "Final stringSyncAttribute value: " + stringSyncAttribute.value());
        LOG_INFO(logger, "Final vectorAttribute size: " + std::to_string(vectorAttribute.value().size()));

        LOG_INFO(logger, "Active threads in strategy: " + std::to_string(strategy.getActiveThreadCount()));
        LOG_INFO(logger, "Queued tasks in strategy: " + std::to_string(strategy.getQueueSize()));

        LOG_INFO(logger, "Complex test completed successfully");
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(logger, "Unexpected exception in main test function: " + std::string(e.what()));
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
        LOG_INFO(logger, "Task " + std::to_string(i) +
                             " submitted. Queue size: " + std::to_string(pool.getQueueSize()) +
                             ", Idle threads: " + std::to_string(pool.getIdleThreads()) +
                             ", Active threads: " + std::to_string(pool.getActiveThreads()));
    }

    for (size_t i = 0; i < results.size(); ++i)
    {
        LOG_INFO(logger, "Task " + std::to_string(i) + " result: " + std::to_string(results[i].get()) +
                             ", Queue size: " + std::to_string(pool.getQueueSize()) +
                             ", Idle threads: " + std::to_string(pool.getIdleThreads()) +
                             ", Active threads: " + std::to_string(pool.getActiveThreads()));
    }

    double avgTime, minTime, maxTime;
    pool.getStats(avgTime, minTime, maxTime);
    std::stringstream ss;
    ss << "Task statistics - Avg time: " << avgTime << "ms, Min time: " << minTime << "ms, Max time: " << maxTime
       << "ms";
    LOG_INFO(logger, ss.str());
    LOG_INFO(logger, "Total tasks processed: " + std::to_string(pool.getTotalTasks()));
}

void testOpenCLWrapper()
{
    try
    {
        auto& ocl = OpenCLWrapper::getInstance();
        ocl.init();

        // 加载内核文件
        std::cout << "Loading kernels..." << std::endl;
        ocl.loadKernelsFromDirectory("kernels");

        // 准备数据
        const size_t dataSize = 1000000;
        std::cout << "Preparing data with size: " << dataSize << std::endl;
        std::vector<float> a(dataSize, 1.0f);
        std::vector<float> b(dataSize, 2.0f);
        std::vector<float> result(dataSize);

        // 创建缓冲区
        std::cout << "Creating buffers..." << std::endl;
        cl_mem bufA = ocl.createBuffer<float>(dataSize, CL_MEM_READ_ONLY);
        cl_mem bufB = ocl.createBuffer<float>(dataSize, CL_MEM_READ_ONLY);
        cl_mem bufC = ocl.createBuffer<float>(dataSize, CL_MEM_WRITE_ONLY);

        // 写入数据
        std::cout << "Writing data to buffers..." << std::endl;
        ocl.writeBuffer(bufA, a);
        ocl.writeBuffer(bufB, b);

        // 配置内核执行参数
        OpenCLWrapper::KernelConfig config;
        config.globalWorkSize = {dataSize};
        config.localWorkSize = {256};
        config.useLocalSize = true;

        // 执行内核
        std::cout << "Executing kernel..." << std::endl;
        ocl.executeKernel("vectorAdd", config,
                          [&]()
                          {
                              auto kernel = ocl.getKernel("vectorAdd");
                              cl_int err;

                              err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufA);
                              if (err != CL_SUCCESS)
                                  throw std::runtime_error("Failed to set kernel arg 0: " + std::to_string(err));

                              err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufB);
                              if (err != CL_SUCCESS)
                                  throw std::runtime_error("Failed to set kernel arg 1: " + std::to_string(err));

                              err = clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufC);
                              if (err != CL_SUCCESS)
                                  throw std::runtime_error("Failed to set kernel arg 2: " + std::to_string(err));
                          });

        // 读取结果
        std::cout << "Reading results..." << std::endl;
        ocl.readBuffer(bufC, result);

        // 验证结果
        std::cout << "Validating results..." << std::endl;
        bool correct = true;
        int errorCount = 0;
        const int maxErrorsToPrint = 10;

        for (size_t i = 0; i < dataSize; ++i)
        {
            float expected = a[i] + b[i];
            if (std::abs(result[i] - expected) > 1e-5f)
            {
                if (errorCount < maxErrorsToPrint)
                {
                    std::cout << "Error at position " << i << ": result = " << result[i] << ", expected = " << expected
                              << std::endl;
                }
                errorCount++;
                correct = false;
            }
        }

        if (correct)
        {
            std::cout << "All computations are correct!" << std::endl;
            // 打印一些示例结果
            std::cout << "\nSample results (first 5 elements):" << std::endl;
            for (size_t i = 0; i < 5 && i < dataSize; ++i)
            {
                std::cout << a[i] << " + " << b[i] << " = " << result[i] << std::endl;
            }
        }
        else
        {
            std::cout << "Total errors found: " << errorCount << std::endl;
        }

        // 输出性能统计
        const auto& stats = ocl.getPerformanceStats();
        for (const auto& [kernelName, stat] : stats)
        {
            std::cout << "\nPerformance statistics for kernel '" << kernelName << "':" << std::endl;
            std::cout << "  Average execution time: " << (stat.averageTime * 1000) << " ms" << std::endl;
            std::cout << "  Min execution time: " << (stat.minTime * 1000) << " ms" << std::endl;
            std::cout << "  Max execution time: " << (stat.maxTime * 1000) << " ms" << std::endl;
            std::cout << "  Total executions: " << stat.executionCount << std::endl;
        }

        // 清理缓冲区
        std::cout << "Cleaning up resources..." << std::endl;
        clReleaseMemObject(bufA);
        clReleaseMemObject(bufB);
        clReleaseMemObject(bufC);

        std::cout << "Program completed successfully!" << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return;
    }
}

int main()
{
    LoggerWrapper logger("Test.txt");

    testStateMachine(logger);
    testThreadPool(logger);

    std::vector<int> numArray(1000);
    std::iota(numArray.begin(), numArray.end(), 0);

    std::for_each(std::execution::par_unseq, numArray.begin(), numArray.end(),
                  [&logger](int num) { LOG_DEBUG(logger, "number: " + std::to_string(num)); });

    // 测试 calculateSpeed 函数
    double distance = 1000.0;
    LOG_INFO(logger, "Speed 1: " + std::to_string(calculateSpeed(distance, SpeedParams1)));
    LOG_INFO(logger, "Speed 2: " + std::to_string(calculateSpeed(distance, SpeedParams2)));
    LOG_INFO(logger, "Speed 3: " + std::to_string(calculateSpeed(distance, SpeedParams3)));

    // 订阅测试
    complexTestSubscriberSystem(logger);

    // OpenCL测试
    testOpenCLWrapper();

    return 0;
}
