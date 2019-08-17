#pragma once
#include <memory>
#include <thread>
#include <mutex>

class EventLoop;

class EventLoopThread {
public:
    EventLoopThread();
    ~EventLoopThread();
    EventLoopThread(const EventLoopThread&) = delete;
    std::shared_ptr<EventLoop> startLoop();
private:
    void run();
private:
    std::shared_ptr<std::thread> thr_ = nullptr;
    std::shared_ptr<EventLoop> loop_ = nullptr;
    std::mutex mtx_;
    std::condition_variable cond_;
};