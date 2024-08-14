#pragma once

#include <cassert>
#include <functional>
#include <list>
#include <memory>
#include <vector>

namespace comm
{

class IInvokeStrategy
{
public:
    virtual ~IInvokeStrategy() = default;
    virtual void invoke(std::function<void()> func) = 0;
};

template<typename... Arguments>
class Subscribable
{
public:
    using Listener = std::function<void(const Arguments&...)>;

    // 前向声明
    class Subscription;
    using SubscriptionPtr = std::shared_ptr<Subscription>;

    Subscribable() = default;
    virtual ~Subscribable()
    {
        m_listeners.clear();
    }

    Subscribable(const Subscribable&) = delete;
    Subscribable& operator=(const Subscribable&) = delete;

    [[nodiscard]] SubscriptionPtr subscribe(Listener listener);

protected:
    void notifySync(const Arguments&... arguments) const;
    void notifyAsync(IInvokeStrategy& strategy, const Arguments&... arguments) const;

private:
    struct IUnsubscriber
    {
        virtual ~IUnsubscriber() = default;
        virtual void unsubscribe() = 0;
    };

    class Subscription : public std::enable_shared_from_this<Subscription>
    {
    public:
        explicit Subscription(Listener listener);
        ~Subscription();

        void invoke(const Arguments&... args);

        std::weak_ptr<IUnsubscriber> m_unsubscriber;

    private:
        Listener m_listener;
    };

    struct SubscriberRecord : IUnsubscriber
    {
        std::weak_ptr<Subscription> listener;
        typename std::list<std::shared_ptr<SubscriberRecord>>::iterator iterator;
        std::list<std::shared_ptr<SubscriberRecord>>* list = nullptr;

        explicit SubscriberRecord(std::weak_ptr<Subscription> l);
        void unsubscribe() override;
    };

    std::list<std::shared_ptr<SubscriberRecord>> m_listeners;
};

// 实现部分
template<typename... Arguments>
typename Subscribable<Arguments...>::SubscriptionPtr Subscribable<Arguments...>::subscribe(Listener listener)
{
    auto subscription = std::make_shared<Subscription>(listener);
    auto record = std::make_shared<SubscriberRecord>(subscription);
    m_listeners.push_back(record);
    record->iterator = std::prev(m_listeners.end());
    record->list = &m_listeners;
    subscription->m_unsubscriber = record;
    return subscription;
}

template<typename... Arguments>
void Subscribable<Arguments...>::notifySync(const Arguments&... arguments) const
{
    std::vector<std::weak_ptr<Subscription>> listenersCopy;
    listenersCopy.reserve(m_listeners.size());
    for (const auto& record : m_listeners)
    {
        listenersCopy.push_back(record->listener);
    }

    for (const auto& weakListener : listenersCopy)
    {
        if (auto listener = weakListener.lock())
        {
            listener->invoke(arguments...);
        }
    }
}

template<typename... Arguments>
void Subscribable<Arguments...>::notifyAsync(IInvokeStrategy& strategy, const Arguments&... arguments) const
{
    for (const auto& record : m_listeners)
    {
        strategy.invoke(
            [weak = record->listener, arguments...]
            {
                if (auto listener = weak.lock())
                {
                    listener->invoke(arguments...);
                }
            });
    }
}

template<typename... Arguments>
Subscribable<Arguments...>::Subscription::Subscription(Listener listener)
    : m_listener(std::move(listener))
{
}

template<typename... Arguments>
Subscribable<Arguments...>::Subscription::~Subscription()
{
    if (auto unsubscriber = m_unsubscriber.lock())
    {
        unsubscriber->unsubscribe();
    }
}

template<typename... Arguments>
void Subscribable<Arguments...>::Subscription::invoke(const Arguments&... args)
{
    m_listener(args...);
}

template<typename... Arguments>
Subscribable<Arguments...>::SubscriberRecord::SubscriberRecord(std::weak_ptr<Subscription> l)
    : listener(std::move(l))
{
}

template<typename... Arguments>
void Subscribable<Arguments...>::SubscriberRecord::unsubscribe()
{
    if (list)
    {
        list->erase(iterator);
        list = nullptr;
    }
}

}  // namespace comm
