#include <iostream>
#include <optional>
#include <string>
#include <boost/filesystem.hpp>

#include "MathFunctions.hpp"
#include "Camera.hpp"
#include "DataSource.hpp"

using namespace std;

using namespace MathFunctions;

using namespace StateMachine;

const std::string testString = "TEST Console Start!";
constexpr double BASE_SPEED_1 = 111.0945;
constexpr double MAX_SPEED_1 = 30;
constexpr double SCALE_DISTANCE_1 = 392.9842;
constexpr double EXPONENT_1 = 1.972242;

constexpr double BASE_SPEED_2 = 90.03563;
constexpr double MAX_SPEED_2 = 60;
constexpr double SCALE_DISTANCE_2 = 314.6796;
constexpr double EXPONENT_2 = 14.54655;


constexpr double BASE_SPEED_3 = 60;
constexpr double MAX_SPEED_3 = 20;
constexpr double SCALE_DISTANCE_3 = 100;
constexpr double EXPONENT_3 = 1.584963;

std::optional<std::string> getTestString()
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

std::optional<std::pair<std::string, std::string>> extractIdAndMessage(const std::string& command)
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
    cout << "strType = " << strType << ", id = " << id << ", MSG = " << message << endl;
    if (command.find("SLOW_DOWN") != std::string::npos)
    {
        cout << "find SLOW_DOWN." << endl;
    }

    return std::make_pair(id, message);
}

std::optional<std::string> replaceCommandWithString(std::string& messageText) {
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
    const auto testString = getTestString();
    cout << testString.value_or("no msg!!!") << endl;
    std::unique_ptr<MathFunction> math;
    const auto result = math->calDividedFunction(7, 0);
    if (result.has_value())
    {
        cout << result.value() << '\n';
    }
    else
    {
        cout << "Invalid input!" << '\n';
    }

    // Test Camera statemachine.
    DataSource dataSource;
    std::shared_ptr<Camera> cam = std::make_shared<Camera>(dataSource);
    cam->initiate();
    cam->process_event(EvShutterFull("enter shooting"));
    cam->process_event(EvShutterRelease("enter NoShooting"));
    cam->process_event(EvConfig("enter config"));

    // Test String
    cout << "\n###################################" << endl;
    std::string inputStr = "in 400 meters, enter the tunnel#SLOW_DWON|2|, please slow down#";

    auto message = replaceCommandWithString(inputStr);

    if (!message)
    {
        std::cerr << "Failed to replace command in the string." << std::endl;
        return -1;
    }

    // Log
    const auto location = source_location::current();
    std::cout << "This file : " << location.file_name()
              << ", line : " << location.line()
              << ", function : " << location.function_name()
              << ", column : " << location.column()
              << '\n';
    std::cout << "Message: " << *message << std::endl;
    std::cout << "Replaced String: " << inputStr << std::endl;

    // Test Algorithm
    cout << "\n###################################" << endl;
    auto safeSpeed1 = BASE_SPEED_1 + (MAX_SPEED_1 - BASE_SPEED_1) / (1 + pow((400.0 / SCALE_DISTANCE_1), EXPONENT_1));

    auto safeSpeed2 = BASE_SPEED_2 + (MAX_SPEED_2 - BASE_SPEED_2) / (1 + pow((400.0 / SCALE_DISTANCE_2), EXPONENT_2));

    auto safeSpeed3 = BASE_SPEED_3 + (MAX_SPEED_3 - BASE_SPEED_3) / (1 + pow((400.0 / SCALE_DISTANCE_3), EXPONENT_3));

    cout << "safeSpeed1 = " << safeSpeed1 << ", safeSpeed2 = " << safeSpeed2 << ", safeSpeed3 = " << safeSpeed3;
}