#include "epoller.h"
#include "../../base/logger.h"
#include <unistd.h>
#include "../channel.h"

#define MAX_EVENT 1024

Epoller::Epoller() :
    Poller(),
    epollfd_(epoll_create(1)) {
    if (epollfd_ == -1) {
        LOG_FATAL << "epoll create failed";
    }   
}

Epoller::~Epoller() {
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
            // if (events[i].filter == EVFILT_TIMER) {
            //     evt |= CHANNEL_TIMER;
            // }
            LOG_TRACE << "active fd:" << id << "|events:" << evt;
            channels_[id]->setActiveEvent(evt);
            ret.push_back(channels_[id]);
        }else if (id == wakeup_channel_->fd()) {
            // wake up
        }
    }
    return ret;
}

void Epoller::wakeup() {
    // struct kevent changes[1];
    // EV_SET(&changes[0], wakeup_channel_->fd(), EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
    // kevent(kq_, changes, 1, NULL, 0, NULL);
}

int Epoller::getTimerFd() {
    return 0;
}
