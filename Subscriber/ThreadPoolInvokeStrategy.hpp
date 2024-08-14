#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "Subscriber.hpp"

namespace comm
{

class ThreadPoolInvokeStrategy : public IInvokeStrategy
{
public:
    explicit ThreadPoolInvokeStrategy(size_t threadCount = std::thread::hardware_concurrency())
        : m_running(true)
        , m_activeThreads(0)
    {
        for (size_t i = 0; i < threadCount; ++i)
        {
            m_threads.emplace_back(&ThreadPoolInvokeStrategy::workerThread, this);
        }
    }

    ~ThreadPoolInvokeStrategy()
    {
        shutdown();
    }

    void invoke(std::function<void()> func) override
    {
        std::packaged_task<void()> task(std::move(func));
        std::future<void> future = task.get_future();

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_taskQueue.emplace(std::move(task));
        }
        m_condition.notify_one();

        // Optionally wait for the task to complete
        // future.wait();
    }

    void shutdown()
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_running = false;
        }
        m_condition.notify_all();
        for (auto& thread : m_threads)
        {
            if (thread.joinable())
            {
                thread.join();
            }
        }
    }

    size_t getQueueSize() const
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        return m_taskQueue.size();
    }

    size_t getActiveThreadCount() const
    {
        return m_activeThreads.load();
    }

private:
    void workerThread()
    {
        while (true)
        {
            std::packaged_task<void()> task;
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_condition.wait(lock, [this] { return !m_running || !m_taskQueue.empty(); });
                if (!m_running && m_taskQueue.empty())
                {
                    return;
                }
                task = std::move(m_taskQueue.front());
                m_taskQueue.pop();
            }
            m_activeThreads++;
            task();
            m_activeThreads--;
        }
    }

    std::vector<std::thread> m_threads;
    std::queue<std::packaged_task<void()>> m_taskQueue;
    mutable std::mutex m_queueMutex;
    std::condition_variable m_condition;
    std::atomic<bool> m_running;
    std::atomic<size_t> m_activeThreads;
};

}  // namespace comm
