// Observer.hpp
#pragma once

#include <algorithm>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <vector>

template<typename... Args>
class Subject
{
public:
    using Observer = std::function<void(Args...)>;
    using ObserverId = size_t;

    ObserverId addObserver(Observer observer)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        ObserverId id = nextObserverId_++;
        observers_.emplace_back(ObserverWrapper{std::move(observer), id});
        return id;
    }

    bool removeObserver(ObserverId id)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        auto it = std::find_if(observers_.begin(), observers_.end(), [id](const auto& obs) { return obs.id == id; });
        if (it != observers_.end())
        {
            observers_.erase(it);
            return true;
        }
        return false;
    }

    void notify(Args... args) const
    {
        std::vector<ObserverWrapper> observersCopy;
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            observersCopy = observers_;
        }

        for (const auto& observer : observersCopy)
        {
            try
            {
                observer.callback(args...);
            }
            catch (const std::exception& e)
            {
                std::cerr << "Exception in observer " << observer.id << ": " << e.what() << std::endl;
            }
            catch (...)
            {
                std::cerr << "Unknown exception in observer " << observer.id << std::endl;
            }
        }
    }

    void clearObservers()
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        observers_.clear();
    }

    size_t observerCount() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return observers_.size();
    }

    std::optional<ObserverId> findObserver(const Observer& observer) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = std::find_if(observers_.begin(), observers_.end(),
                               [&](const auto& obs) { return obs.callback.target_type() == observer.target_type(); });
        if (it != observers_.end())
        {
            return it->id;
        }
        return std::nullopt;
    }

private:
    struct ObserverWrapper
    {
        Observer callback;
        ObserverId id;
    };

    mutable std::shared_mutex mutex_;
    std::vector<ObserverWrapper> observers_;
    ObserverId nextObserverId_ = 0;
};

//===========================Usage example=========================================
// #include <iostream>

// class WeatherStation : public Subject<float, float>
// {
// public:
//     void setMeasurements(float temperature, float humidity)
//     {
//         notify(temperature, humidity);
//     }
// };

// int main()
// {
//     WeatherStation station;

//     auto observer1 = [](float temp, float humidity)
//     {
//         std::cout << "Observer 1: Temperature: " << temp << "C, Humidity: " << humidity << "%" << std::endl;
//     };

//     auto observer2 = [](float temp, float humidity)
//     {
//         std::cout << "Observer 2: Temperature: " << temp << "C, Humidity: " << humidity << "%" << std::endl;
//         throw std::runtime_error("Test exception");
//     };

//     auto id1 = station.addObserver(observer1);
//     auto id2 = station.addObserver(observer2);

//     std::cout << "Observer count: " << station.observerCount() << std::endl;

//     station.setMeasurements(25.5, 60);

//     if (auto foundId = station.findObserver(observer1); foundId)
//     {
//         std::cout << "Found observer1 with id: " << *foundId << std::endl;
//         station.removeObserver(*foundId);
//     }

//     std::cout << "Observer count after removal: " << station.observerCount() << std::endl;

//     station.setMeasurements(26.0, 61);

//     station.clearObservers();

//     std::cout << "Observer count after clear: " << station.observerCount() << std::endl;

//     return 0;
// }
