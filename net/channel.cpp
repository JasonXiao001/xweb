#include "channel.h"
#include "../base/logger.h"

Channel::Channel(int fd, bool isListen) :
    fd_(fd),
    isListenFd_(isListen),
    events_(0),
    lastEvents_(0),
    repeat_(false),
    interval_(0) {

}

Channel::Channel() :
    fd_(0),
    isListenFd_(false),
    events_(0),
    lastEvents_(0),
    repeat_(false),
    interval_(0) {

}

void Channel::handleEvent() {
    LOG_TRACE << "active " << active_events_;
    if (isListenFd_) {
        connHandler_();
    }else if (active_events_ & CHANNEL_READ) {
        readHandler_();
    }else if (active_events_ & CHANNEL_WRITE) {
        writeHandler_();
    }else if (active_events_ & CHANNEL_TIMER) {
        LOG_TRACE << "on timer";
        timerHandler_();
    }
}
