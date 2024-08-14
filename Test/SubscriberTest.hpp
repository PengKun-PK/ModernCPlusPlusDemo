#pragma once

#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <thread>
#include "Subscriber.hpp"

namespace comm
{
namespace test
{

class TestInvokeStrategy : public IInvokeStrategy
{
public:
    void invoke(std::function<void()> func) override
    {
        func();
    }
};

template<typename... Arguments>
class TestSubscribable : public Subscribable<Arguments...>
{
public:
    using Subscribable<Arguments...>::Subscribable;

    void testNotifySync(const Arguments&... arguments) const
    {
        this->notifySync(arguments...);
    }

    void testNotifyAsync(IInvokeStrategy& strategy, const Arguments&... arguments) const
    {
        this->notifyAsync(strategy, arguments...);
    }
};

class SubscribableTest : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

    TestInvokeStrategy testStrategy;
};

}  // namespace test
}  // namespace comm
