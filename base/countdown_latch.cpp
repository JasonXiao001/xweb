#include "countdown_latch.h"

CountDownLatch::CountDownLatch(int count) : 
    mutex_(),
    cond_(),
    count_(count)
{
}

void CountDownLatch::wait()
{
    std::unique_lock<std::mutex> lck(mutex_);
    while (count_ > 0)
    {
        cond_.wait(lck);
    }
}

void CountDownLatch::countDown()
{
    std::lock_guard<std::mutex> lck(mutex_);
    --count_;
    if (count_ == 0)
    {
        cond_.notify_all();
    }
}

int CountDownLatch::getCount() const
{
    std::lock_guard<std::mutex> lck(mutex_);
    return count_;
}

