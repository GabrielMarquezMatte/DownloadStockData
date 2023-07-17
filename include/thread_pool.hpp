#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class thread_pool
{
public:
    thread_pool(const size_t thread_count)
    {
        for (size_t i = 0; i < thread_count; ++i)
        {
            threads.emplace_back([this]()
            {
                while (true)
                {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(this->queue_mutex);
                        this->condition.wait(lock, [this]()
                        {
                            return this->stop || !this->tasks.empty();
                        });
                        if (this->stop && this->tasks.empty())
                            return;
                        task = std::move(this->tasks.front());
                        this->tasks.pop();
                    }
                    task();
                }
            });
        }
    }

    ~thread_pool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread &thread : threads)
            thread.join();
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args)
    {
        using return_type = std::invoke_result_t<F, Args...>;
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                throw std::runtime_error("enqueue on stopped thread_pool");
            tasks.emplace([task]()
            {
                (*task)();
            });
        }
        condition.notify_one();
        return result;
    }

private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
};