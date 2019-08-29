#pragma once
#include "channel.h"
#include "socket.h"

class EventLoop;
class InetAddress;

class Acceptor {
public:
    typedef std::function<void (int sockfd, const InetAddress&)> NewConnectionCallback;

    Acceptor(EventLoop* loop, int port);
    ~Acceptor();
    Acceptor(const Acceptor&) = delete;
    void listen();
    bool listenning() const { return listenning_; }
    void setNewConnectionCallback(const NewConnectionCallback& cb)
    { new_conn_cb_ = cb; }

private:
    void handleConn();

    int listenfd_;
    bool listenning_;
    EventLoop *loop_;
    Channel accept_channel_;
    NewConnectionCallback new_conn_cb_;
    Socket accept_sock_;

};