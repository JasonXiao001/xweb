#include "kqueue_poller.h"
#include <sys/event.h>
#include <iostream>
#include "channel.h"
#include "../base/logger.h"

#define MAX_EVENT 64
#define WAKEUP_FD 50000
#define TIMER_FD_BEGIN 40000


KqueuePoller::KqueuePoller() :
    Poller(),
    kq_(kqueue()) {
    static int fd = WAKEUP_FD;
    wakeup_channel_ = std::make_shared<Channel>(fd++);
    struct kevent changes[1];
    EV_SET(&changes[0], wakeup_channel_->fd(), EVFILT_USER, EV_ADD | EV_CLEAR, 0, 0, NULL);
    kevent(kq_, changes, 1, NULL, 0, NULL);
    LOG_INFO << "create poller|wakeup fd:" << wakeup_channel_->fd();
}


void KqueuePoller::update(Channel *channel) {
    int fd = channel->fd();
    int last = channel->lastEvents();
    int evt = channel->events();
    LOG_TRACE << "fd:" << fd << "|events:" << channel->events() << "|last_events:" << channel->lastEvents();
    struct kevent changes[3];
    int n = 0;
    if ((evt & CHANNEL_READ) || (last & CHANNEL_READ) || (evt & CHANNEL_CONN)) {
        EV_SET(&changes[n++], fd, EVFILT_READ, ((evt & CHANNEL_READ) ||  (evt & CHANNEL_CONN)) ? EV_ADD|EV_ENABLE : EV_DISABLE, 0, 0, NULL);           
    }
    if ((last & CHANNEL_WRITE) || (evt & CHANNEL_WRITE)) {
        EV_SET(&changes[n++], fd, EVFILT_WRITE, evt & CHANNEL_WRITE ? EV_ADD|EV_ENABLE : EV_DISABLE, 0, 0, NULL);           
    }
    if (evt & CHANNEL_TIMER) {
        EV_SET(&changes[n++], fd, EVFILT_TIMER, channel->repeat() ? EV_ADD : (EV_ADD | EV_ONESHOT), 0, channel->interval(), NULL);
    }
    int ret = kevent(kq_, changes, n, NULL, 0, NULL);
    if (ret != 0) {
        LOG_ERROR << "kevent failed " << ret;
    }
    channel->updateEvent();
    channels_[fd] = channel;
}

void KqueuePoller::remove(Channel *channel) {
    int fd = channel->fd();
    LOG_TRACE << "remove " << fd;
    if (channels_.find(fd) != channels_.end()) {
        struct kevent changes[2];
        int n = 0;
        int evt = channel->events();
        if (evt & CHANNEL_READ) {
            EV_SET(&changes[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);           
        }
        if (evt & CHANNEL_WRITE) {
            EV_SET(&changes[n++], fd, CHANNEL_WRITE, EV_DELETE, 0, 0, NULL);           
        }
        if (evt & CHANNEL_TIMER) {
            EV_SET(&changes[n++], fd, EVFILT_TIMER, EV_DELETE, 0, 0, NULL);           
        }
        kevent(kq_, changes, n, NULL, 0, NULL);
        channels_.erase(fd);
    }
}

int KqueuePoller::getTimerFd() {
    static int fd = 40000;
    fd++;
    if (fd >= WAKEUP_FD) {
        fd = TIMER_FD_BEGIN;
    }
    return fd;
}

std::vector<Channel*> KqueuePoller::poll() {
    struct kevent events[MAX_EVENT];
    int nev = kevent(kq_, NULL, 0, events, MAX_EVENT, NULL);
    LOG_TRACE << nev << " events happened";
    std::vector<Channel*> ret;
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
            if (events[i].filter == EVFILT_TIMER) {
                evt |= CHANNEL_TIMER;
            }
            channels_[id]->setActiveEvent(evt);
            ret.push_back(channels_[id]);
        }else if (id == wakeup_channel_->fd()) {
            // wake up
        }
    }
    return ret;
}


void KqueuePoller::wakeup() {
    struct kevent changes[1];
    EV_SET(&changes[0], wakeup_channel_->fd(), EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
    kevent(kq_, changes, 1, NULL, 0, NULL);
}

