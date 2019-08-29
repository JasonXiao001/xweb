#pragma once
#include <memory>
#include "inet_address.h"
#include "callback.h"
#include <map>

using std::string;

class Acceptor;
class EventLoop;
class TcpConnection;

class TcpServer {
public:
    TcpServer(EventLoop *loop, int port);
    ~TcpServer();
    TcpServer(const TcpServer&) = delete;
    void start();
    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }
private:
    void newConnection(int sockfd, const InetAddress& peerAddr);
    void removeConnection(const TcpConnectionPtr& conn);

private:
    typedef std::map<string, TcpConnectionPtr> ConnectionMap;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    int port_;
    int nextConnId_;
    EventLoop* loop_;  // the acceptor loop
    std::unique_ptr<Acceptor> acceptor_;
    bool started_;
    ConnectionMap conn_map_;
};