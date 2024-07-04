#include <iostream>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <atomic>
#include <type_traits>
#include <chrono>

class ThreadPool {
public:
    explicit ThreadPool(size_t numThreads)
        : stop(false), idleThreads(numThreads), activeThreads(0), totalTasks(0) {
        for(size_t i = 0; i < numThreads; ++i) {
            workers.emplace_back([this] {
                while(true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });
                        if(stop && tasks.empty()) return;
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    idleThreads--;
                    activeThreads++;
                    task();
                    activeThreads--;
                    idleThreads++;
                    if(stop && tasks.empty()) return;
                }
            });
        }
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if(stop) throw std::runtime_error("enqueue on stopped ThreadPool");
            tasks.emplace([this, task](){
                auto start = std::chrono::high_resolution_clock::now();
                (*task)();
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double, std::milli> elapsed = end - start;
                updateStats(elapsed.count());
            });
        }
        totalTasks++;
        condition.notify_one();
        return res;
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for(std::thread &worker: workers) {
            worker.join();
        }
    }

    size_t getIdleThreads() const { return idleThreads; }
    size_t getActiveThreads() const { return activeThreads; }
    size_t getQueueSize() const {
        std::unique_lock<std::mutex> lock(queue_mutex);
        return tasks.size();
    }
    size_t getTotalTasks() const { return totalTasks; }

    void getStats(double& avgTime, double& minTime, double& maxTime) const {
        std::unique_lock<std::mutex> lock(stats_mutex);
        avgTime = totalTaskTime / (totalTasks - tasks.size());
        minTime = minTaskTime;
        maxTime = maxTaskTime;
    }

    void resetStats() {
        std::unique_lock<std::mutex> lock(stats_mutex);
        totalTaskTime = 0;
        minTaskTime = std::numeric_limits<double>::max();
        maxTaskTime = 0;
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    mutable std::mutex queue_mutex;
    mutable std::mutex stats_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
    std::atomic<size_t> idleThreads;
    std::atomic<size_t> activeThreads;
    std::atomic<size_t> totalTasks;

    double totalTaskTime = 0;
    double minTaskTime = std::numeric_limits<double>::max();
    double maxTaskTime = 0;

    void updateStats(double taskTime) {
        std::unique_lock<std::mutex> lock(stats_mutex);
        totalTaskTime += taskTime;
        minTaskTime = std::min(minTaskTime, taskTime);
        maxTaskTime = std::max(maxTaskTime, taskTime);
    }
};