#pragma once

#include "poller.h"

class KqueuePoller : public Poller {
public:
    KqueuePoller();
    virtual void update(Channel *channel) override;
    virtual void remove(Channel *channel) override;
    virtual int getTimerFd() override;
    virtual std::vector<Channel*> poll() override;
    virtual void wakeup() override;
private:
    int kq_;
};