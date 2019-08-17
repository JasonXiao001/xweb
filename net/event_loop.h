#pragma once
#include <memory>
#include <thread>
#include <cstdint>
#include <mutex>
#include <functional>
#include <vector>

class Poller;
class Channel;

class EventLoop {
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    void loop();
    void quit();
    void runInLoop(Functor&& cb);
    void queueInLoop(Functor&& cb);
    bool isLoopThread() const {
        return thread_id_ == std::this_thread::get_id();
    }
    std::thread::id getThreadId() const {
        return thread_id_;
    }
    void addToPoller(std::shared_ptr<Channel> channel, uint64_t timeout = 0);
    void updatePoller(std::shared_ptr<Channel> channel, uint64_t timeout = 0);
    void removeFromPoller(std::shared_ptr<Channel> channel);
    void doPendingFunctors();
private:
    void wakeup();
private:
    bool running_;
    bool calling_pending_;
    std::shared_ptr<Poller> poller_;
    std::thread::id thread_id_;
    std::vector<Functor> pending_functors_;
    std::mutex mtx_;
    int wakeup_fd_;
    std::shared_ptr<Channel> wakeup_channel_;
};