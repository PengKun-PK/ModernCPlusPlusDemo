#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include <string>

using namespace testing;

TEST(Test1, test1)
{
    std::string testString1 = "Hello";
    EXPECT_EQ(testString1, "Hello");
}