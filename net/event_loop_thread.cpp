#include "event_loop_thread.h"
#include "event_loop.h"

EventLoopThread::EventLoopThread() :
    mtx_(),
    cond_() {

}

EventLoopThread::~EventLoopThread() {

}


std::shared_ptr<EventLoop> EventLoopThread::startLoop() {
    thr_ = std::make_shared<std::thread>(&EventLoopThread::run, this);
    {
        std::unique_lock<std::mutex> lck(mtx_);
        while(loop_ == nullptr) {
            cond_.wait(lck);
        }
    }
    return loop_;
}

void EventLoopThread::run() {
    {
        std::lock_guard<std::mutex> lck(mtx_);
        loop_ = std::make_shared<EventLoop>();
        cond_.notify_one();
    }
    loop_->loop();
    loop_ = nullptr;
}