#pragma once

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

    const ValueType& value() const
    {
        return m_value;
    }

    bool setValue(const ValueType& value)
    {
        if (m_value == value && !m_notifyIfNotChanged)
        {
            return false;
        }
        m_value = value;
        notify();
        return true;
    }

protected:
    virtual void notify()
    {
        this->notifySync(m_value);
    }

private:
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

    const ValueType& value() const
    {
        return m_value;
    }

    bool setValue(const ValueType& value)
    {
        if (m_value == value && !m_notifyIfNotChanged)
        {
            return false;
        }
        m_value = value;
        notify();
        return true;
    }

protected:
    virtual void notify()
    {
        this->notifyAsync(m_strategy, m_value);
    }

private:
    IInvokeStrategy& m_strategy;
    ValueType m_value;
    bool m_notifyIfNotChanged;
};

class Subscriptions
{
public:
    void add(typename Subscribable<>::SubscriptionPtr subscription)
    {
        m_subscriptions.push_back(std::move(subscription));
    }

    template<typename SubscribableType>
    void subscribe(SubscribableType& subscribable, typename SubscribableType::Listener listener)
    {
        add(subscribable.subscribe(std::move(listener)));
    }

    void unsubscribe()
    {
        m_subscriptions.clear();
    }

    size_t size() const
    {
        return m_subscriptions.size();
    }

private:
    std::vector<typename Subscribable<>::SubscriptionPtr> m_subscriptions;
};

}  // namespace comm
