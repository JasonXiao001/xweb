#pragma once

#include "event_loop.h"
#include "channel.h"
#include "event_loop.h"
#include "thread_pool.h"
#include <memory>


class HttpServer
{
public:
    HttpServer(std::shared_ptr<EventLoop> loop, int thread_num, int port);
    void start();
    void handNewConn();
    
private:
    std::shared_ptr<EventLoop> loop_;
    std::unique_ptr<ThreadPool> thread_pool_;
    bool started_;
    std::shared_ptr<Channel> accept_channel_;
    int port_;
    int listen_fd_;
    static const int MAXFDS = 100000;
};

