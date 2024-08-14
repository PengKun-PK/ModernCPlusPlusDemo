#pragma once

#include <atomic>
#include <functional>
#include <list>
#include <memory>
#include <mutex>
#include <shared_mutex>
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
class Subscribable : public std::enable_shared_from_this<Subscribable<Arguments...>>
{
public:
    using Listener = std::function<void(const Arguments&...)>;

    class Subscription;
    using SubscriptionPtr = std::shared_ptr<Subscription>;

    Subscribable() = default;
    virtual ~Subscribable() = default;

    Subscribable(const Subscribable&) = delete;
    Subscribable& operator=(const Subscribable&) = delete;

    [[nodiscard]] SubscriptionPtr subscribe(Listener listener);

protected:
    void notifySync(const Arguments&... arguments) const;
    void notifyAsync(IInvokeStrategy& strategy, const Arguments&... arguments) const;

private:
    class Subscription : public std::enable_shared_from_this<Subscription>
    {
    public:
        explicit Subscription(Listener listener, std::weak_ptr<Subscribable> parent);
        ~Subscription();

        void invoke(const Arguments&... args);
        void unsubscribe();

    private:
        Listener m_listener;
        std::weak_ptr<Subscribable> m_parent;
        std::atomic<bool> m_isActive{true};
    };

    void removeSubscription(const Subscription* sub);

    mutable std::shared_mutex m_mutex;
    std::list<std::weak_ptr<Subscription>> m_subscriptions;
};

// Implementation
template<typename... Arguments>
typename Subscribable<Arguments...>::SubscriptionPtr Subscribable<Arguments...>::subscribe(Listener listener)
{
    auto subscription = std::make_shared<Subscription>(std::move(listener), this->weak_from_this());
    {
        std::unique_lock lock(m_mutex);
        m_subscriptions.push_back(subscription);
    }
    return subscription;
}

template<typename... Arguments>
void Subscribable<Arguments...>::notifySync(const Arguments&... arguments) const
{
    std::vector<std::shared_ptr<Subscription>> activeSubscriptions;
    {
        std::shared_lock lock(m_mutex);
        activeSubscriptions.reserve(m_subscriptions.size());
        for (const auto& weakSub : m_subscriptions)
        {
            if (auto sub = weakSub.lock())
            {
                activeSubscriptions.push_back(std::move(sub));
            }
        }
    }

    for (const auto& sub : activeSubscriptions)
    {
        sub->invoke(arguments...);
    }
}

template<typename... Arguments>
void Subscribable<Arguments...>::notifyAsync(IInvokeStrategy& strategy, const Arguments&... arguments) const
{
    std::vector<std::shared_ptr<Subscription>> activeSubscriptions;
    {
        std::shared_lock lock(m_mutex);
        activeSubscriptions.reserve(m_subscriptions.size());
        for (const auto& weakSub : m_subscriptions)
        {
            if (auto sub = weakSub.lock())
            {
                activeSubscriptions.push_back(std::move(sub));
            }
        }
    }

    for (const auto& sub : activeSubscriptions)
    {
        strategy.invoke([sub, args = std::make_tuple(arguments...)]
                        { std::apply([&sub](const auto&... params) { sub->invoke(params...); }, args); });
    }
}

template<typename... Arguments>
void Subscribable<Arguments...>::removeSubscription(const Subscription* sub)
{
    std::unique_lock lock(m_mutex);
    m_subscriptions.remove_if(
        [sub](const std::weak_ptr<Subscription>& weakSub)
        {
            auto sharedSub = weakSub.lock();
            return !sharedSub || sharedSub.get() == sub;
        });
}

template<typename... Arguments>
Subscribable<Arguments...>::Subscription::Subscription(Listener listener, std::weak_ptr<Subscribable> parent)
    : m_listener(std::move(listener))
    , m_parent(std::move(parent))
{
}

template<typename... Arguments>
Subscribable<Arguments...>::Subscription::~Subscription()
{
    unsubscribe();
}

template<typename... Arguments>
void Subscribable<Arguments...>::Subscription::invoke(const Arguments&... args)
{
    if (m_isActive)
    {
        m_listener(args...);
    }
}

template<typename... Arguments>
void Subscribable<Arguments...>::Subscription::unsubscribe()
{
    bool expected = true;
    if (m_isActive.compare_exchange_strong(expected, false))
    {
        if (auto parent = m_parent.lock())
        {
            parent->removeSubscription(this);
        }
    }
}

}  // namespace comm
