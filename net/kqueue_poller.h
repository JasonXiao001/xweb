#pragma once

#include "poller.h"

class KqueuePoller : public Poller {
public:
    KqueuePoller();
    virtual void add(std::shared_ptr<Channel> channel, uint64_t timeout = 0) override;
    virtual void update(std::shared_ptr<Channel> channel) override;
    virtual void remove(std::shared_ptr<Channel> channel) override;
    virtual std::vector<std::shared_ptr<Channel>> poll() override;

private:
    int kq_;
};