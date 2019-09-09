#include "event_loop.h"
#include <assert.h>
#include "poller.h"
#include "channel.h"
#include "../base/rio.h"
#include "../base/logger.h"
#include <memory>

__thread static EventLoop* t_loopInThisThread = nullptr;

EventLoop* EventLoop::getEventLoopOfCurrentThread()
{
    return t_loopInThisThread;
}

EventLoop::EventLoop() :
    running_(false),
    calling_pending_(false),
    poller_(createPoller()),
    thread_id_(std::this_thread::get_id()) {
    LOG_DEBUG << "EventLoop created " << this << " in thread " << thread_id_;
    if (t_loopInThisThread) {
        LOG_FATAL << "Another EventLoop " << t_loopInThisThread
                << " exists in this thread " << thread_id_;
    }
    else {
        t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop() {
    LOG_DEBUG << "EventLoop " << this << " of thread " << thread_id_ << " destructs in thread " << std::this_thread::get_id();
    quit();
}


void EventLoop::loop() {
    assert(isLoopThread());
    assert(!running_);
    running_ = true;
    LOG_TRACE << "EventLoop " << this << " start";
    while(running_) {
        auto ret = poller_->poll();
        LOG_TRACE << "active events " << ret.size();
        for (auto it = ret.begin(); it != ret.end(); ++it) {
            (*it)->handleEvent();
        }
        doPendingFunctors();

    }
}

void EventLoop::quit() {
    running_ = false;
    if (!isLoopThread()) {
        wakeup();
    }
}

void EventLoop::updateChannel(Channel *channel) {
    runInLoop([this, channel](){
        poller_->update(channel);
    });
}

void EventLoop::removeChannel(Channel *channel) {
    runInLoop([this, channel](){
        poller_->remove(channel);
    });
}

void EventLoop::runInLoop(Functor&& cb)
{
    if (isLoopThread())
        cb();
    else
        queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb) {
    {
        std::lock_guard<std::mutex> lck(mtx_);
        pending_functors_.emplace_back(std::move(cb));
    }
    if (!isLoopThread() || calling_pending_) {
        wakeup();
    }
}

void EventLoop::doPendingFunctors()
{
    std::vector<Functor> functors;
    calling_pending_ = true;

    {
        std::lock_guard<std::mutex> lck(mtx_);
        functors.swap(pending_functors_);
    }

    for (size_t i = 0; i < functors.size(); ++i)
        functors[i]();
    calling_pending_ = false;
}

void EventLoop::wakeup() {
    poller_->wakeup();
}

int EventLoop::runAfter(int64_t delay, Functor &&cb) {
    int fd = poller_->getTimerFd();
    runInLoop([this, delay, cb, fd]() mutable{
        auto channel = std::unique_ptr<Channel>(new Channel);
        channel->setRepeat(false);
        channel->setInterval(delay);
        channel->setEvent(CHANNEL_TIMER);
        channel->setTimerHandler(std::move(cb));
        channel->setFd(fd);
        poller_->update(channel.get());
        timers_[fd] = std::move(channel);
        LOG_TRACE << "add fd " << fd;
    });
    return fd;
}

int EventLoop::runEvery(int64_t interval, Functor &&cb) {
    int fd = poller_->getTimerFd();
    runInLoop([this, interval, cb, fd]() mutable{
        auto channel = std::unique_ptr<Channel>(new Channel);
        channel->setRepeat(true);
        channel->setInterval(interval);
        channel->setEvent(CHANNEL_TIMER);
        channel->setTimerHandler(std::move(cb));
        channel->setFd(fd);
        poller_->update(channel.get());
        timers_[fd] = std::move(channel);
        LOG_TRACE << "add fd " << fd;
    });
    return fd;
}

void EventLoop::cancel(int id) {
    runInLoop([this, id](){
        if (timers_.find(id) != timers_.end()) {
            poller_->remove(timers_[id].get());
            timers_.erase(id);
        }
    });
}
