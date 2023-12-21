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

    std::string id = command.substr(firstDelim + 1, lastDelim - firstDelim - 1);
    std::string message = command.substr(lastDelim + 1);
    cout << "id = " << id << ", MSG = " << message << endl;

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
    MathFunction math;
    const auto result = math.calDividedFunction(7, 0);
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
    Camera cam(dataSource);
    cam.initiate();
    cam.process_event(EvShutterFull("enter shooting"));
    cam.process_event(EvShutterRelease("enter NoShooting"));
    cam.process_event(EvConfig("enter config"));

    // Test String
    cout << "\n###################################" << endl;
    std::string inputStr = "in 400 meters, enter the tunnel#SLOW_DOWN|1|, please slow down#";

    auto message = replaceCommandWithString(inputStr);

    if (!message)
    {
        std::cerr << "Failed to replace command in the string." << std::endl;
        return -1;
    }

    std::cout << "Message: " << *message << std::endl;
    std::cout << "Replaced String: " << inputStr << std::endl;
}