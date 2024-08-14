#include "SubscriberTest.hpp"

namespace comm
{
namespace test
{

void SubscribableTest::SetUp()
{
    // 在每个测试用例开始前执行的设置
}

void SubscribableTest::TearDown()
{
    // 在每个测试用例结束后执行的清理
}

// 基本订阅和同步通知测试
TEST_F(SubscribableTest, BasicSubscribeAndNotifySync)
{
    TestSubscribable<int, std::string> subject;
    int callCount = 0;
    std::string lastMessage;

    auto subscription = subject.subscribe(
        [&](int num, const std::string& msg)
        {
            callCount++;
            lastMessage = msg;
        });

    subject.testNotifySync(42, "Hello");
    EXPECT_EQ(callCount, 1);
    EXPECT_EQ(lastMessage, "Hello");

    subject.testNotifySync(100, "World");
    EXPECT_EQ(callCount, 2);
    EXPECT_EQ(lastMessage, "World");
}

// 异步通知测试
TEST_F(SubscribableTest, NotifyAsync)
{
    TestSubscribable<int> subject;
    std::atomic<int> callCount{0};

    auto subscription = subject.subscribe([&](int num) { callCount++; });

    subject.testNotifyAsync(testStrategy, 42);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    EXPECT_EQ(callCount, 1);
}

// 多个订阅者测试
TEST_F(SubscribableTest, MultipleSubscribers)
{
    TestSubscribable<int> subject;
    std::atomic<int> totalCalls{0};

    auto sub1 = subject.subscribe([&](int num) { totalCalls++; });
    auto sub2 = subject.subscribe([&](int num) { totalCalls++; });

    subject.testNotifySync(42);
    EXPECT_EQ(totalCalls, 2);
}

// 其他测试用例类似地修改...

// 订阅者抛出异常测试
TEST_F(SubscribableTest, SubscriberThrowsException)
{
    TestSubscribable<int> subject;
    int callCount = 0;

    auto sub1 = subject.subscribe([&](int num) { callCount++; });
    auto sub2 = subject.subscribe([&](int num) { throw std::runtime_error("Test exception"); });
    auto sub3 = subject.subscribe([&](int num) { callCount++; });

    EXPECT_THROW({ subject.testNotifySync(42); }, std::runtime_error);

    EXPECT_EQ(callCount, 1);
}

// 测试不同参数类型
TEST_F(SubscribableTest, DifferentArgumentTypes)
{
    TestSubscribable<int, std::string, double> subject;
    int intValue = 0;
    std::string strValue;
    double doubleValue = 0.0;

    auto subscription = subject.subscribe(
        [&](int i, const std::string& s, double d)
        {
            intValue = i;
            strValue = s;
            doubleValue = d;
        });

    subject.testNotifySync(42, "Test", 3.14);
    EXPECT_EQ(intValue, 42);
    EXPECT_EQ(strValue, "Test");
    EXPECT_DOUBLE_EQ(doubleValue, 3.14);
}

}  // namespace test
}  // namespace comm
