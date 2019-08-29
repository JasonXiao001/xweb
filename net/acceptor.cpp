#include "acceptor.h"
#include "event_loop.h"
#include "inet_address.h"
#include "../base/logger.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>

#define LISTENQ 1024


Acceptor::Acceptor(EventLoop* loop, int port) :
    loop_(loop),
    listenning_(false),
    new_conn_cb_(nullptr),
    listenfd_(socket(AF_INET, SOCK_STREAM, 0)),
    accept_sock_(listenfd_),
    accept_channel_(listenfd_, true) {
    int optval = 1;
    struct sockaddr_in serveraddr;
    if (listenfd_ < 0) 
        LOG_FATAL << "create socket error";
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    accept_sock_.setReuseAddr(true);
    accept_sock_.setReusePort(true);
    accept_sock_.bindAddress(InetAddress(serveraddr));
    accept_channel_.setEvent(CHANNEL_CONN);
    accept_channel_.setConnHandler(std::bind(&Acceptor::handleConn, this));
    loop_->updateChannel(&accept_channel_);
}

Acceptor::~Acceptor() {
    loop_->removeChannel(&accept_channel_);
    close(listenfd_);
}

void Acceptor::listen() {
    loop_->runInLoop([this]() {
        accept_sock_.listen();
        listenning_ = true;
    });
}

void Acceptor::handleConn() {
    struct sockaddr_in clientaddr;
    InetAddress addr(clientaddr);
    int connfd = accept_sock_.accept(addr);
    LOG_INFO << "on new connect|fd:" << connfd;
    if (new_conn_cb_) {
        new_conn_cb_(connfd, addr);
    }

}