#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <atomic>

#include "core/types.hpp"

namespace utils
{

class ThreadPool
{

public:
    ThreadPool(usize threads = std::thread::hardware_concurrency() - 1)
    {
        threads = std::max(usize(1), threads);

        for (usize i = 0; i < threads; i++) {
            m_workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(m_queueMutex);
                        m_condition.wait(lock, [this] {
                            return m_stop || !m_tasks.empty();
                        });

                        if (m_stop && m_tasks.empty()) {
                            return;
                        }

                        task = std::move(m_tasks.front());
                        m_tasks.pop();
                    }

                    task();
                }
            });
        }
    }

    template<typename F, typename... Args>
    auto enqueue(F &&f, Args &&... args) -> std::future<
        typename std::invoke_result<F, Args...>::type
    >
    {
        using return_type = typename std::invoke_result<F, Args...>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<return_type> result = task->get_future();

        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            if (m_stop) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            m_tasks.emplace([task]() { (*task)(); });
        }

        m_condition.notify_one();

        return result;
    }

    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            m_stop = true;
        }

        m_condition.notify_all();

        for (std::thread &worker : m_workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    usize size() const
    {
        return m_workers.size();
    }

    usize queueSize() const
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        return m_tasks.size();
    }
    
private:
    std::vector<std::thread> m_workers;
    std::queue<std::function<void()>> m_tasks;

    mutable std::mutex m_queueMutex;
    std::condition_variable m_condition;

    bool m_stop = false;

};

} // namespace utils