#pragma once
#include <memory>
#include <functional>

#define CHANNEL_CONN (0x01)
#define CHANNEL_READ (0x02)
#define CHANNEL_WRITE (0x04)
#define CHANNEL_TIMER (0x08)

class EventLoop;

class Channel {
private:
    typedef std::function<void()> CallBack;

public:
    Channel();
    Channel(int fd, bool isListen = false);
    Channel(const Channel &) = delete;
    int fd() const {
        return fd_;
    }
    void setFd(int fd) {
        fd_ = fd;
    }
    int events() const {
        return events_;
    }
    bool repeat() const {
        return repeat_;
    }
    void setRepeat(bool repeat) {
        repeat_ = repeat;
    }
    int64_t interval() const {
        return interval_;
    }
    void setInterval(int64_t interval) {
        interval_ = interval;
    }
    void handleEvent();
    void setReadHandler(CallBack &&readHandler)
    {
        readHandler_ = readHandler;
    }
    void setWriteHandler(CallBack &&writeHandler)
    {
        writeHandler_ = writeHandler;
    }
    void setErrorHandler(CallBack &&errorHandler)
    {
        errorHandler_ = errorHandler;
    }
    void setConnHandler(CallBack &&connHandler)
    {
        connHandler_ = connHandler;
    }
    void setTimerHandler(CallBack &&timerHandler)
    {
        timerHandler_ = timerHandler;
    }
    void setEvent(int event) {
        events_ = event;
    }
    int lastEvents() const {
        return lastEvents_;
    }
    void updateEvent() {
        lastEvents_ = events_;
    }
    void setActiveEvent(int event) {
        active_events_ = event;
    }
    int getActiveEvent() const {
        return active_events_;
    }
private:
    int events_;
    int active_events_;
    int lastEvents_;
    int fd_;
    bool isListenFd_;
    int64_t interval_;
    bool repeat_;
    CallBack readHandler_;
    CallBack writeHandler_;
    CallBack errorHandler_;
    CallBack connHandler_;
    CallBack timerHandler_;
};