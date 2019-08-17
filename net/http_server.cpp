#include "http_server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <iostream>
#include "http_data.h"

#define LISTENQ 1024

static 
int open_listenfd(int port) {
    int listenfd, optval = 1;
    struct sockaddr_in serveraddr;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int)) < 0)
        return -1;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)port);
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        return -1;
    if (listen(listenfd, LISTENQ) < 0)
        return -1;
    return listenfd;
}

HttpServer::HttpServer(std::shared_ptr<EventLoop> loop, int thread_num, int port) :
    loop_(loop),
    port_(port),
    thread_pool_(new ThreadPool(loop, thread_num)),
    listen_fd_(open_listenfd(port)) {
    if (listen_fd_ > 0) {
        accept_channel_ = std::make_shared<Channel>(listen_fd_);
        accept_channel_->setFd(listen_fd_);
    }else {
        std::cout << "listen error" << std::endl;
    }
}

void HttpServer::start() {
    thread_pool_->start();
    accept_channel_->setReadHandler(std::bind(&HttpServer::handNewConn, this));
    accept_channel_->setEvent(CHANNEL_CONN);
    loop_->addToPoller(accept_channel_, true);
    started_ = true;
}

void HttpServer::handNewConn() {
    std::cout << "on new connection" << std::endl;
    struct sockaddr_in clientaddr;
    size_t clientlen = sizeof(clientaddr);
    int connfd = accept(listen_fd_, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen);
    auto next_loop = thread_pool_->getNextLoop();
    // next_loop->addToPoller();
    // std::shared_ptr<HttpData> req(new HttpData(next_loop, connfd));
    // TODO 内存泄漏
    auto data = new HttpData(next_loop, connfd);
    data->newRequest();
    // next_loop->queueInLoop(std::bind(&HttpData::newEvent, req));
}