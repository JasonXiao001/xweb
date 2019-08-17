#pragma once
#include <memory>
#include <cstdint>
#include <vector>
#include <map>

class Channel;

class Poller {
public:
    virtual ~Poller() {}
    virtual void add(std::shared_ptr<Channel> channel, uint64_t timeout = 0) = 0;
    virtual void update(std::shared_ptr<Channel> channel) = 0;
    virtual void remove(std::shared_ptr<Channel> channel) = 0;
    virtual std::vector<std::shared_ptr<Channel>> poll() = 0;
protected:
    std::map<int, std::shared_ptr<Channel>> channels_;
};


Poller *createPoller();