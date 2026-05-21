#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <thread>
#include <queue>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

constexpr size_t num_threads{20};

//COPYPASTED FROM
//https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

class ThreadPool {
private:
    void threadLoop();

    bool should_terminate{false};          
    std::mutex queue_mutex;                
    std::condition_variable mutex_condition;
    std::vector<std::jthread> threads;
    std::queue<std::function<void()>> jobs;
public:
    void start();
    void queueJob(const std::function<void()>& job);
    void stop();
    bool isBusy();
};

#endif // THREADPOOL_H