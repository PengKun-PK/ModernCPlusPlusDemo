#include <iostream>
#include <optional>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <execution>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "Singleton.hpp"
#include "MathFunctions.hpp"
#include "Camera.hpp"
#include "DataSource.hpp"
#include "ILogger.hpp"
#include "StateMachine.hpp"

using namespace std;

using namespace MathFunctions;

using namespace StateMachine;

using namespace Trace;

inline const std::string testString = "TEST Console Start!";
inline constexpr double BASE_SPEED_1 = 111.0945;
inline constexpr double MAX_SPEED_1 = 30;
inline constexpr double SCALE_DISTANCE_1 = 392.9842;
inline constexpr double EXPONENT_1 = 1.972242;

inline constexpr double BASE_SPEED_2 = 90.03563;
inline constexpr double MAX_SPEED_2 = 60;
inline constexpr double SCALE_DISTANCE_2 = 314.6796;
inline constexpr double EXPONENT_2 = 14.54655;

inline constexpr double BASE_SPEED_3 = 60;
inline constexpr double MAX_SPEED_3 = 20;
inline constexpr double SCALE_DISTANCE_3 = 100;
inline constexpr double EXPONENT_3 = 1.584963;

[[nodiscard]] std::optional<std::string> getTestString()
{
    if (testString.empty())
    {
        return std::nullopt;
    }
    else
    {
        return testString;
    }
}

// 分隔符定义为constexpr字符数组
constexpr char DelimiterStart = '#';
constexpr char DelimiterMiddle = '|';

[[nodiscard("返回值很重要")]] std::optional<std::pair<std::string, std::string>> extractIdAndMessage(const std::string& command)
{
    const auto firstDelim = command.find(DelimiterMiddle);
    const auto lastDelim = command.rfind(DelimiterMiddle);

    if (firstDelim == std::string::npos || firstDelim == lastDelim)
    {
        return std::nullopt;
    }

    std::string strType = command.substr(0, firstDelim);
    std::string id = command.substr(firstDelim + 1, lastDelim - firstDelim - 1);
    std::string message = command.substr(lastDelim + 1);

    return std::make_pair(id, message);
}

[[nodiscard]] std::optional<std::string> replaceCommandWithString(std::string& messageText) {
    const auto startPos = messageText.find(DelimiterStart);
    const auto endPos = messageText.rfind(DelimiterStart);

    if (startPos == std::string::npos || endPos == std::string::npos || startPos == endPos)
    {
        return std::nullopt;
    }

    auto extracted = extractIdAndMessage(messageText.substr(startPos + 1, endPos - startPos - 1));

    if (!extracted.has_value())
    {
        return std::nullopt;
    }

    if (0)
    {
        messageText = messageText.substr(0, startPos);
    }
    else
    {
        messageText.replace(startPos, endPos - startPos + 1, extracted->second);
    }

    return extracted->second;
}

int main()
{
    // Trace
    auto now = std::chrono::system_clock::now(); // 获取当前时间点
    auto in_time_t = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&in_time_t), "%Y_%m_%d_%H_%M_%S"); // 格式化日期和时间
    std::string currentDateTime = ss.str(); // 转换为字符串

    auto logger = ILogger::getLogger("Test.txt");
    logger->setLevel(LogLevel::debug);
    logger->setFilePath("Logs/" + currentDateTime + ".txt");

    const auto testString = getTestString();
    logger->info(testString.value_or("no msg!!!"));
    const auto& math = Instance<MathFunction>();
    const auto result = math.calDividedFunction(7, 0);
    if (result.has_value())
    {
        std::string numStr = boost::lexical_cast<std::string>(result.value());
        logger->info(numStr);
    }
    else
    {
        logger->error("Invalid result.");
    }

    // Test Camera statemachine.
    auto& dataSource = Instance<DataSource>();
    std::shared_ptr<Camera> cam = std::make_shared<Camera>(dataSource);
    cam->initiate();
    cam->process_event(EvShutterFull("enter shooting"));
    cam->process_event(EvShutterRelease("enter NoShooting"));
    cam->process_event(EvConfig("enter config"));

    // Test StataMachine
    auto& sm = Instance<MyStateMachine<void>>();

    logger->info("Starting state machine\n");
    sm.start();

    logger->info("Sending Event1\n");
    sm.process(StateMachine_<void>::Event1{42});

    logger->info("Sending Event2\n");
    sm.process(StateMachine_<void>::Event2{"Hello"});

    logger->info("Sending Event3\n");
    sm.process(StateMachine_<void>::Event3{});

    logger->info("Sending Event2 (internal transition)\n");
    sm.process(StateMachine_<void>::Event2{"Internal"});
}