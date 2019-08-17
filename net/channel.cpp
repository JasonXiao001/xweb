#include "channel.h"

Channel::Channel(int fd, bool isListen) :
    fd_(fd),
    isListenFd_(isListen),
    events_(0),
    lastEvents_(0) {

}

void Channel::handleEvent() {
    if (isListenFd_) {
        connHandler_();
    }else if (active_events_ & CHANNEL_READ) {
        readHandler_();
    }else if (active_events_ & CHANNEL_WRITE) {
        writeHandler_();
    }
}