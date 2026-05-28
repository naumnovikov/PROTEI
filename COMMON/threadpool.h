#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

constexpr size_t num_threads{20};

// COPYPASTED FROM
// https://stackoverflow.com/questions/15752659/thread-pooling-in-c11

class ThreadPool {
 private:
  void threadLoop();

  std::atomic<bool> should_terminate{false};
  std::mutex queue_mutex;
  std::condition_variable mutex_condition;
  std::vector<std::thread> threads;
  std::queue<std::function<void()>> jobs;

 public:
  inline bool isTerminating() const { return should_terminate.load(); }

  void start();
  void queueJob(const std::function<void()>& job);
  void stop();
  bool isBusy();
};

#endif  // THREADPOOL_H