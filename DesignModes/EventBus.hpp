#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <exception>
#include <iostream>

template<typename EventBaseType>
class EventBus
{
public:
    // Unique identifier for each subscription
    using SubscriptionId = size_t;

    // Template function to subscribe to an event type
    template<typename EventType, typename Callback>
    SubscriptionId subscribe(Callback&& callback)
    {
        static_assert(std::is_base_of_v<EventBaseType, EventType>,
                      "EventType must inherit from EventBaseType");
        static_assert(std::is_invocable_r_v<void, Callback, const EventType&>,
                      "Callback must be invocable with const EventType&");

        std::unique_lock lock(mutex_);
        auto& subscribers = subscribers_[std::type_index(typeid(EventType))];
        SubscriptionId id = nextSubscriptionId_++;
        subscribers.emplace_back(Subscriber{
            id,
            [cb = std::forward<Callback>(callback)](const EventBaseType* event)
            {
                try
                {
                    cb(*static_cast<const EventType*>(event));
                }
                catch (const std::exception& e)
                {
                    std::cerr << "Exception in event callback: " << e.what() << std::endl;
                }
                catch (...)
                {
                    std::cerr << "Unknown exception in event callback" << std::endl;
                }
            }
        });
        return id;
    }

    // Function to publish an event
    template<typename EventType>
    void publish(const EventType& event) const
    {
        static_assert(std::is_base_of_v<EventBaseType, EventType>,
                      "EventType must inherit from EventBaseType");

        std::shared_lock lock(mutex_);
        if (auto it = subscribers_.find(std::type_index(typeid(EventType))); it != subscribers_.end())
        {
            // Create a copy of subscribers to allow for safe iteration
            auto subscribersCopy = it->second;
            lock.unlock();  // Release the lock before calling callbacks

            for (const auto& subscriber : subscribersCopy)
            {
                subscriber.callback(&event);
            }
        }
    }

    // Function to unsubscribe all callbacks for a specific event type
    template<typename EventType>
    void unsubscribeAll()
    {
        static_assert(std::is_base_of_v<EventBaseType, EventType>,
                      "EventType must inherit from EventBaseType");

        std::unique_lock lock(mutex_);
        subscribers_.erase(std::type_index(typeid(EventType)));
    }

    // Function to unsubscribe a specific subscription
    void unsubscribe(SubscriptionId id)
    {
        std::unique_lock lock(mutex_);
        for (auto& [_, subscribers] : subscribers_)
        {
            subscribers.erase(
                std::remove_if(subscribers.begin(), subscribers.end(),
                    [id](const auto& subscriber) { return subscriber.id == id; }),
                subscribers.end()
            );
        }
    }

private:
    // Structure to hold subscriber information
    struct Subscriber
    {
        SubscriptionId id;
        std::function<void(const EventBaseType*)> callback;
    };

    using SubscriberList = std::vector<Subscriber>;

    mutable std::shared_mutex mutex_;  // Mutex for thread-safety
    std::unordered_map<std::type_index, SubscriberList> subscribers_;  // Map of event types to subscribers
    SubscriptionId nextSubscriptionId_ = 0;  // Counter for generating unique subscription IDs
};

//=============================================Usage example===================================================

// struct Event { virtual ~Event() = default; };
// struct UserCreatedEvent : Event { std::string username; };

// int main()
// {
//     EventBus<Event> bus;

//     auto id1 = bus.subscribe<UserCreatedEvent>([](const UserCreatedEvent& e) {
//         std::cout << "User created: " << e.username << std::endl;
//     });

//     auto id2 = bus.subscribe<UserCreatedEvent>([](const UserCreatedEvent& e) {
//         throw std::runtime_error("Test exception");
//     });

//     bus.publish(UserCreatedEvent{"John"});  // This will print "User created: John" and an exception message

//     bus.unsubscribe(id2);  // Unsubscribe the throwing callback

//     bus.publish(UserCreatedEvent{"Jane"});  // This will only print "User created: Jane"

//     bus.unsubscribeAll<UserCreatedEvent>();  // Unsubscribe all UserCreatedEvent callbacks

//     bus.publish(UserCreatedEvent{"Bob"});  // This won't print anything

//     return 0;
// }