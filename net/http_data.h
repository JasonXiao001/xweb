#pragma once
#include <memory>

class EventLoop;
class Channel;

class HttpData {
public:
    HttpData(std::shared_ptr<EventLoop> loop, int fd);
    HttpData(const HttpData&) = delete;
    void newRequest();
private:
    void handleRead();
    void handleWrite();
private:
    EventLoop *loop_;
    std::shared_ptr<Channel> channel_;
    int fd_;
};