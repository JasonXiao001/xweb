#include "event_loop.h"
#include "poller.h"
#include <iostream>
#include "channel.h"
#include "rio.h"

static int createFd() {
    static int fd = 10000;
    fd++;
    return fd;
}


EventLoop::EventLoop() :
    running_(false),
    calling_pending_(false),
    poller_(createPoller()),
    wakeup_fd_(createFd()),
    thread_id_(std::this_thread::get_id()),
    wakeup_channel_(new Channel(wakeup_fd_)) {
    poller_->add(wakeup_channel_);
}

EventLoop::~EventLoop() {

}


void EventLoop::loop() {
    assert(isLoopThread());
    assert(!running_);
    running_ = true;
    while(running_) {
        auto ret = poller_->poll();
        std::cout << ret.size() << " current thread " << std::this_thread::get_id() << std::endl;

        for (auto it = ret.begin(); it != ret.end(); ++it) {
            (*it)->handleEvent();
        }
        doPendingFunctors();

    }
}

void EventLoop::quit() {

}

void EventLoop::addToPoller(std::shared_ptr<Channel> channel, uint64_t timeout) {
    // std::cout << "poller addr " << poller_.get() << " current thread " << std::this_thread::get_id() << std::endl;
    poller_->add(channel, timeout);
}

void EventLoop::updatePoller(std::shared_ptr<Channel> channel, uint64_t timeout) {
    std::cout <<  std::this_thread::get_id() << running_ << std::endl;
    poller_->update(channel);
}

void EventLoop::removeFromPoller(std::shared_ptr<Channel> channel) {
    poller_->remove(channel);
}

void EventLoop::runInLoop(Functor&& cb)
{
    if (isLoopThread())
        cb();
    else
        queueInLoop(std::move(cb));
}

void EventLoop::queueInLoop(Functor&& cb)
{
    {
        std::lock_guard<std::mutex> lck(mtx_);
        pending_functors_.emplace_back(std::move(cb));
    }
    if (!isLoopThread() || calling_pending_)
        wakeup();
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

void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = rio_writen(wakeup_fd_, (char*)(&one), sizeof one);
}