#pragma once
#include <memory>
#include <cstdint>
#include <vector>
#include <map>

class Channel;

class Poller {
public:
    virtual ~Poller() {}
    virtual void update(Channel *channel) = 0;
    virtual void remove(Channel *channel) = 0;
    virtual int getTimerFd() = 0;
    virtual std::vector<Channel*> poll() = 0;
    virtual void wakeup() = 0;
protected:
    std::map<int, Channel*> channels_;
    std::shared_ptr<Channel> wakeup_channel_;
};


Poller *createPoller();