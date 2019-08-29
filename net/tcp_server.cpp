#include "tcp_server.h"
#include "acceptor.h"
#include "event_loop.h"
#include "../base/logger.h"
#include "tcp_connection.h"
#include <functional>


TcpServer::TcpServer(EventLoop *loop, int port) :
    loop_(loop),
    port_(port),
    nextConnId_(0),
    acceptor_(new Acceptor(loop, port)),
    started_(false) {
    acceptor_->setNewConnectionCallback(std::bind(&TcpServer::newConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer() {
    assert(loop_->isLoopThread());
    LOG_TRACE << "server " << port_ << " destructing";
    for (auto& item : conn_map_) {
        TcpConnectionPtr conn(item.second);
        item.second.reset();
        conn->shutdown();
    }
}

void TcpServer::start() {
    if (!started_) {
        started_ = true;
        acceptor_->listen();       
    }

}

void TcpServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    assert(loop_->isLoopThread());
    char buf[64];
    snprintf(buf, sizeof buf, "%d#%d", port_, nextConnId_++);
    LOG_INFO << "new connection [" << buf << "] from " << peerAddr.toString();
    TcpConnectionPtr conn(new TcpConnection(loop_, buf, sockfd, peerAddr));
    conn_map_[std::string(buf)] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TcpServer::removeConnection, this, std::placeholders::_1));
    conn->connectEstablished();
}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    LOG_INFO << "remove " << conn->name();
    size_t n = conn_map_.erase(conn->name());
}