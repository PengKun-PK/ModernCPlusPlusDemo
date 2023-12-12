#include "TestCamera.hpp"

#include <string>

using namespace testing;

namespace StateMachine
{

CameraTest::CameraTest()
{

}

CameraTest::~CameraTest()
{

}

TEST_F(CameraTest, test1)
{
    std::string testString1 = "Hello";
    EXPECT_EQ(testString1, "Hello");
}

}