#pragma once
#include <future>
#include <thread>
#include <functional>
#include <concurrentqueue/concurrentqueue.h>

class ThreadPool final
{
private:
    std::vector<std::jthread> m_threads;
    moodycamel::ConcurrentQueue<std::function<void()>> m_tasks;
    std::atomic_bool m_done = false;
public:
    ThreadPool(size_t numThreads = std::thread::hardware_concurrency()/2);
    ~ThreadPool();
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>
    {
        using return_type = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        auto future = task->get_future();
        m_tasks.enqueue([task]() { (*task)(); });
        return future;
    }
};