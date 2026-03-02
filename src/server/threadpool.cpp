#include "threadpool.hpp"

ThreadPool::ThreadPool(size_t threads) : stop(false) 
{
    // Launch the requested number of worker threads
    for (size_t i = 0; i < threads; ++i) 
    {
        workers.emplace_back([this] 
        {
            // Each worker runs an infinite loop
            for (;;) 
            {
                std::function<void()> task;

                {
                    // Step 1a: Lock the queue
                    std::unique_lock<std::mutex> lock(this->queueMutex);

                    // Step 1b: Wait until there is a task or stop flag is set
                    this->condition.wait(lock, [this] {
                        return this->stop || !this->tasks.empty();
                        });

                    // Step 1c: If stop flag is set and no tasks remain, exit thread
                    if (this->stop && this->tasks.empty())
                        return;

                    // Step 1d: Otherwise, pop a task from the queue
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                // Step 1e: Execute the task outside the lock
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() 
{
    {
        // Step 3a: Lock the queue
        std::unique_lock<std::mutex> lock(queueMutex);

        // Step 3b: Set stop flag to true
        stop = true;
    }

    // Step 3c: Notify all worker threads so they wake up
    condition.notify_all();

    // Step 3d: Join all threads to ensure clean shutdown
    for (std::thread& worker : workers) 
    {
        if (worker.joinable()) 
        {
            worker.join();
        }
    }
}

