#include "thread_pool.h"
#include "event_loop.h"
#include "event_loop_thread.h"
#include "../base/logger.h"
#include <iostream>

ThreadPool::ThreadPool(std::shared_ptr<EventLoop> mainloop, size_t maxn) :
    maxn_(maxn),
    next_(0),
    mainloop_(mainloop),
    started_(false) {

}

void ThreadPool::start() {
    LOG_INFO << "start thread pool|thread:" << maxn_;
    assert(mainloop_->isLoopThread());
    started_ = true;
    for (int i = 0; i < maxn_; ++i) {
        std::shared_ptr<EventLoopThread> t(new EventLoopThread());
        threads_.push_back(t);
        loops_.push_back(t->startLoop());
        LOG_INFO << "create thread " << loops_.back()->getThreadId() << " " << loops_.back().get();
    }
}

std::shared_ptr<EventLoop> ThreadPool::getNextLoop() {
    assert(mainloop_->isLoopThread());
    std::shared_ptr<EventLoop> loop = mainloop_;
    if (!loops_.empty()) {
        loop = loops_[next_];
        next_ = (next_ + 1) % maxn_;
    }
    return loop;
}