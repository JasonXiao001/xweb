#include "kqueue_poller.h"
#include <sys/event.h>
#include <iostream>
#include "channel.h"

#define MAX_EVENT 64

KqueuePoller::KqueuePoller() :
    Poller(),
    kq_(kqueue()) {

    }

void KqueuePoller::add(std::shared_ptr<Channel> channel, uint64_t timeout) {
    int fd = channel->getFd();
    struct kevent changes[2];
    int n = 0;
    int ev = channel->getEvent();
    if ((ev & CHANNEL_CONN) || (ev & CHANNEL_READ)) {
        EV_SET(&changes[n++], fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
    }
    if (ev & CHANNEL_WRITE) {
        EV_SET(&changes[n++], fd, EVFILT_WRITE, EV_ADD, 0, 0, NULL);
    }
    kevent(kq_, changes, n, NULL, 0, NULL);
    channel->updateEvent();
    channels_[fd] = channel;
}

void KqueuePoller::update(std::shared_ptr<Channel> channel) {

    int fd = channel->getFd();
    if (channels_.find(fd) != channels_.end()) {
        struct kevent changes[2];
        int n = 0;
        int last = channel->getLastEvent();
        int evt = channel->getEvent();
        if ((evt & CHANNEL_READ) || (last & CHANNEL_READ)) {
            std::cout << "update read " << (evt & CHANNEL_READ ? EV_ADD|EV_ENABLE : EV_DISABLE) << std::endl;
            EV_SET(&changes[n++], fd, EVFILT_READ, evt & CHANNEL_READ ? EV_ADD|EV_ENABLE : EV_DISABLE, 0, 0, NULL);           
        }
        if ((last & CHANNEL_WRITE) || (evt & CHANNEL_WRITE)) {
            std::cout << "update write " << (evt & CHANNEL_WRITE ? EV_ADD|EV_ENABLE : EV_DISABLE) << std::endl;
            EV_SET(&changes[n++], fd, EVFILT_WRITE, evt & CHANNEL_WRITE ? EV_ADD|EV_ENABLE : EV_DISABLE, 0, 0, NULL);           
        }
        int ret = kevent(kq_, changes, n, NULL, 0, NULL);
        if (ret != 0) {
            std::cout << "kevent failed" <<std::endl;
        }
        channel->updateEvent();
    }else {
        std::cout << "can not find channel" << std::endl;
    }
}

void KqueuePoller::remove(std::shared_ptr<Channel> channel) {
    int fd = channel->getFd();
    if (channels_.find(fd) != channels_.end()) {
        struct kevent changes[2];
        int n = 0;
        int evt = channel->getEvent();
        if (evt & CHANNEL_READ) {
            EV_SET(&changes[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);           
        }
        if (evt & CHANNEL_WRITE) {
            EV_SET(&changes[n++], fd, CHANNEL_WRITE, EV_DELETE, 0, 0, NULL);           
        }
        kevent(kq_, changes, n, NULL, 0, NULL);
        channels_.erase(fd);
    }
}

std::vector<std::shared_ptr<Channel>> KqueuePoller::poll() {
    struct kevent events[MAX_EVENT];
    struct timespec timespec;
    timespec.tv_nsec = 0;
    timespec.tv_sec = 60;
    int nev = kevent(kq_, NULL, 0, events, MAX_EVENT, &timespec);
    std::vector<std::shared_ptr<Channel>> ret;
    for (int i = 0; i < nev; ++i) {
        int id = events[i].ident;
        if (channels_.find(id) != channels_.end()) {
            int evt = 0;
            if (events[i].filter == EVFILT_READ) {
                evt |= CHANNEL_READ;
            }
            if (events[i].filter == EVFILT_WRITE) {
                evt |= CHANNEL_WRITE;
            }
            channels_[id]->setActiveEvent(evt);
            ret.push_back(channels_[id]);
        }
    }
    return ret;
}