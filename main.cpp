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
    cam.process_event(EvShutterHalf("enter half shooting"));
    cam.process_event(EvShutterRelease("enter NoShooting"));
    cam.process_event(EvConfig("enter config"));
}