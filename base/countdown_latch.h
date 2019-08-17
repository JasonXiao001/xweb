#pragma once

#include <mutex>
#include <condition_variable>

class CountDownLatch
{
public:
    explicit CountDownLatch(int count);
    ~CountDownLatch() = default;
    CountDownLatch(const CountDownLatch&) = delete;
    void wait();
    void countDown();
    int getCount() const;

private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    int count_;
};
