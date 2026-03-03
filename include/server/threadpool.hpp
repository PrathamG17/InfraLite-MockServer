#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queueMutex;
    std::condition_variable condition;
    bool stop;

public:
    // Constructor: launch threads
    explicit ThreadPool(size_t threads);

    // Enqueue a new task
    template<class F>
    void enqueue(F&& f) 
    {
        {
            // Step 2a: Lock the queue
            std::unique_lock<std::mutex> lock(queueMutex);

            // Step 2b: If pool is stopped, throw an exception
            if (stop)
            {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            // Step 2c: Add the new task to the queue
            tasks.emplace(std::forward<F>(f));
        }

        // Step 2d: Notify one waiting worker thread
        condition.notify_one();
    }


    // Destructor: join all threads
    ~ThreadPool();
};

#endif // THREADPOOL_HPP
