#include "../include/thread_pool.hpp"

ThreadPool::ThreadPool(size_t numThreads)
{
    for (size_t i = 0; i < numThreads; ++i)
    {
        m_threads.emplace_back([this]
        {
            while (!m_done)
            {
                std::function<void()> task;
                while(!m_tasks.try_dequeue(task) && !m_done) ;
                if (task) task();
            }
        });
    }
}

ThreadPool::~ThreadPool()
{
    m_done = true;
    for (auto& thread : m_threads)
    {
        if (thread.joinable()) thread.join();
    }
}