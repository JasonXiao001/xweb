#pragma once
#include <memory>
#include <vector>

class EventLoop;
class EventLoopThread;

class ThreadPool {
public:
    ThreadPool(std::shared_ptr<EventLoop> mainloop, size_t maxn);
    void start();
    std::shared_ptr<EventLoop> getNextLoop();
private:
    size_t maxn_, next_;
    bool started_;
    std::shared_ptr<EventLoop> mainloop_;
    std::vector<std::shared_ptr<EventLoopThread>> threads_;
    std::vector<std::shared_ptr<EventLoop>> loops_;
};
