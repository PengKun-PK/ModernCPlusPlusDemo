#pragma once

#include "Subscriber.hpp"

namespace comm
{

template<typename... Arguments>
class EventSync : public Subscribable<Arguments...>
{
public:
    using Interface = Subscribable<Arguments...>;
    using Sync = EventSync<Arguments...>;

    EventSync() = default;

    void notify(const Arguments&... arguments) const
    {
        this->notifySync(arguments...);
    }
};

template<typename... Arguments>
class Event : public Subscribable<Arguments...>
{
public:
    using Interface = Subscribable<Arguments...>;
    using Sync = EventSync<Arguments...>;

    explicit Event(IInvokeStrategy& strategy)
        : m_strategy(strategy)
    {
    }

    void notify(const Arguments&... arguments) const
    {
        this->notifyAsync(m_strategy, arguments...);
    }

private:
    IInvokeStrategy& m_strategy;
};

}  // namespace comm
