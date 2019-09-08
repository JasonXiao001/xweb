#include "epoller.h"
#include <sys/eventfd.h>
#include "../../base/logger.h"
#include <unistd.h>
#include <sys/timerfd.h>
#include "../channel.h"

#define MAX_EVENT 1024

static void wakeupRead(int fd) {
    uint64_t one = 1;
    ssize_t n = read(fd, &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
    }
}


Epoller::Epoller() :
    Poller(),
    epollfd_(epoll_create(1)) {
    if (epollfd_ == -1) {
        LOG_FATAL << "epoll create failed";
    }
    int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (efd < 0) {
        LOG_FATAL << "Failed in eventfd";
    }
    wakeup_channel_ = std::make_shared<Channel>(efd);
    wakeup_channel_->setEvent(CHANNEL_READ);
    wakeup_channel_->setReadHandler(std::bind(wakeupRead, efd));
    update(wakeup_channel_.get());
}

Epoller::~Epoller() {
    remove(wakeup_channel_.get());
    close(wakeup_channel_->fd());
    close(epollfd_);
}

void Epoller::update(Channel *channel) {
    int fd = channel->fd();
    int last = channel->lastEvents();
    int evt = channel->events();
    LOG_TRACE << "fd:" << fd << "|events:" << channel->events() << "|last_events:" << channel->lastEvents();
    struct epoll_event event;
    event.data.fd = fd;
    if ((evt & CHANNEL_READ) || (evt & CHANNEL_CONN)) {
        event.events |= EPOLLIN;
    }
    if (evt & CHANNEL_WRITE) {
        event.events |= EPOLLOUT;
    }
    if (evt & CHANNEL_TIMER) {
        struct itimerspec new_value;
        struct timespec now;
        new_value.it_value.tv_sec = now.tv_sec;
        new_value.it_value.tv_nsec = now.tv_nsec;
        new_value.it_interval.tv_sec = 0;
        new_value.it_interval.tv_nsec = channel->interval()*1000000;
        if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &new_value, NULL) == -1) {
            LOG_ERROR << "timer set time error";
            return;
        }  
    }
    bool modify = channels_.find(fd) != channels_.end();
    if(epoll_ctl(epollfd_, modify?EPOLL_CTL_MOD:EPOLL_CTL_ADD, fd, &event) < 0) {
        LOG_ERROR << "epoll_ctl error ";
    }else {
        channel->updateEvent();
        channels_[fd] = channel;
    }

}

void Epoller::remove(Channel *channel) {
    int fd = channel->fd();
    LOG_TRACE << "remove fd:" << fd;
    if (channels_.find(fd) != channels_.end()) {
        if(epoll_ctl(epollfd_, EPOLL_CTL_DEL, fd, NULL) < 0) {
            LOG_ERROR << "epoll_ctl error ";
        }else {
            channels_.erase(fd);
        }
    }
}

std::vector<Channel*> Epoller::poll() {
    struct epoll_event events[MAX_EVENT];
    int nev = ::epoll_wait(epollfd_, events, MAX_EVENT, -1);
    LOG_TRACE << nev << " events happened";
    std::vector<Channel*> ret;
    for (int i = 0; i < nev; ++i) {
        int id = events[i].data.fd;
        if (channels_.find(id) != channels_.end()) {
            int evt = 0;
            if (events[i].events & EPOLLIN) {
                evt |= CHANNEL_READ;
            }
            if (events[i].events== EPOLLOUT) {
                evt |= CHANNEL_WRITE;
            }
            if (channels_[id]->events() & CHANNEL_TIMER) {
                evt |= CHANNEL_TIMER;
            }
            LOG_TRACE << "active fd:" << id << "|events:" << evt;
            channels_[id]->setActiveEvent(evt);
            ret.push_back(channels_[id]);
            // if ((channels_[id]->events() & CHANNEL_TIMER) && !channels_[id]->repeat()) {
            //     remove(channels_[id]);
            // }
        }
    }
    return ret;
}

void Epoller::wakeup() {
    uint64_t one = 1;
    ssize_t n = write(wakeup_channel_->fd(), &one, sizeof one);
    if (n != sizeof one) {
        LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
    }
}

int Epoller::getTimerFd() {
    int fd = timerfd_create(CLOCK_REALTIME, 0);
    if (fd < 0) {
        LOG_FATAL << "create timer fd error";
    }
    return fd;
}
