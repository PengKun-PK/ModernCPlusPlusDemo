#pragma once

#include <mutex>
#include "Subscriber.hpp"

namespace comm
{

template<typename ValueType>
class AttributeSync : public Subscribable<ValueType>
{
public:
    using Interface = Subscribable<ValueType>;
    using Sync = AttributeSync<ValueType>;

    AttributeSync(const ValueType& defaultValue = ValueType(), bool notifyIfNotChanged = false)
        : m_value(defaultValue)
        , m_notifyIfNotChanged(notifyIfNotChanged)
    {
    }

    ValueType value() const
    {
        std::shared_lock lock(m_mutex);
        return m_value;
    }

    bool setValue(const ValueType& value)
    {
        std::unique_lock lock(m_mutex);
        if (m_value == value && !m_notifyIfNotChanged)
        {
            return false;
        }
        m_value = value;
        lock.unlock();
        notify();
        return true;
    }

protected:
    virtual void notify()
    {
        this->notifySync(value());
    }

private:
    mutable std::shared_mutex m_mutex;
    ValueType m_value;
    bool m_notifyIfNotChanged;
};

template<typename ValueType>
class Attribute : public Subscribable<ValueType>
{
public:
    using Interface = Subscribable<ValueType>;
    using Sync = AttributeSync<ValueType>;

    Attribute(IInvokeStrategy& strategy, const ValueType& defaultValue = ValueType(), bool notifyIfNotChanged = false)
        : m_strategy(strategy)
        , m_value(defaultValue)
        , m_notifyIfNotChanged(notifyIfNotChanged)
    {
    }

    ValueType value() const
    {
        std::shared_lock lock(m_mutex);
        return m_value;
    }

    bool setValue(const ValueType& value)
    {
        std::unique_lock lock(m_mutex);
        if (m_value == value && !m_notifyIfNotChanged)
        {
            return false;
        }
        m_value = value;
        lock.unlock();
        notify();
        return true;
    }

protected:
    virtual void notify()
    {
        this->notifyAsync(m_strategy, value());
    }

private:
    IInvokeStrategy& m_strategy;
    mutable std::shared_mutex m_mutex;
    ValueType m_value;
    bool m_notifyIfNotChanged;
};

class Subscriptions
{
public:
    void add(typename Subscribable<>::SubscriptionPtr subscription)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_subscriptions.push_back(std::move(subscription));
    }

    template<typename SubscribableType>
    void subscribe(SubscribableType& subscribable, typename SubscribableType::Listener listener)
    {
        add(subscribable.subscribe(std::move(listener)));
    }

    void unsubscribe()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_subscriptions.clear();
    }

    size_t size() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_subscriptions.size();
    }

private:
    mutable std::mutex m_mutex;
    std::vector<typename Subscribable<>::SubscriptionPtr> m_subscriptions;
};

}  // namespace comm
