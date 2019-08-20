#pragma once
#include <memory>
#include <thread>
#include <cstdint>
#include <mutex>
#include <functional>
#include <vector>
#include <map>
#include <cstdint>

class Poller;
class Channel;
class Timer;

class EventLoop {
public:
    typedef std::function<void()> Functor;

    EventLoop();
    ~EventLoop();
    EventLoop(const EventLoop&) = delete;
    void loop();
    void quit();
    void runInLoop(Functor&& cb);
    bool isLoopThread() const {
        return thread_id_ == std::this_thread::get_id();
    }
    std::thread::id getThreadId() const {
        return thread_id_;
    }
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    int runEvery(int64_t interval, Functor &&cb);
    int runAfter(int64_t delay, Functor &&cb);
    void cancel(int id);
    static EventLoop *getEventLoopOfCurrentThread();
private:
    void wakeup();
    void doPendingFunctors();
    void queueInLoop(Functor&& cb);

private:
    bool running_;
    bool calling_pending_;
    std::unique_ptr<Poller> poller_;
    std::thread::id thread_id_;
    std::vector<Functor> pending_functors_;
    std::map<int, std::unique_ptr<Channel>> timers_;
    std::mutex mtx_;
};