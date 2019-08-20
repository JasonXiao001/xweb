#include "http_data.h"
#include "channel.h"
#include "event_loop.h"
#include "../base/rio.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

#define MAXLINE 1024

int read_requesthdrs(rio_t *rp) {
    char buf[MAXLINE];
    rio_readlineb(rp, buf, MAXLINE);
    int count = 0;
    while(strcmp(buf, "\r\n")) {
        ssize_t ret = rio_readlineb(rp, buf, MAXLINE);
        if (ret == 0) {
            return 1;
        }else {
            printf("%s", buf);
        }
    }
    return 0;
}

void get_filetype(const char *filename, char *filetype) {
    if (strstr(filename, ".html")) {
        strcpy(filetype, "text/html");
    }
    else if (strstr(filename, ".gif")) {
        strcpy(filetype, "image/gif");
    }
    else if (strstr(filename, ".png")) {
        strcpy(filetype, "image/png");
    }
    else if (strstr(filename, "image/jpeg")) {
        strcpy(filetype, "image/jpeg");
    }
    else if (strstr(filename, ".mp4")) {
        strcpy(filetype, "video/mp4");
    }
    else {
        strcpy(filetype, "text/plain");
    }
}

void serve_static(int fd, const char *filename, int filesize) {
    int srcfd;
    char *srcp, filetype[MAXLINE], buf[MAXLINE];

    get_filetype(filename, filetype);
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n", buf, filesize);
    sprintf(buf, "%sContent-type: %s\r\n\r\n", buf, filetype);
    rio_writen(fd, buf, strlen(buf));
    // printf("%s", buf);
    srcfd = open(filename, O_RDONLY, 0);
    while(rio_readn(srcfd, buf, MAXLINE)) {
        rio_writen(fd, buf, MAXLINE);
    }
    close(srcfd);
}

HttpData::HttpData(std::shared_ptr<EventLoop> loop, int fd) :
    loop_(loop.get()),
    fd_(fd),
    channel_(new Channel(fd)) {

    std::cout << "new http data " << loop_ << " " << fd << std::endl;
    channel_->setReadHandler(std::bind(&HttpData::handleRead, this));
    channel_->setWriteHandler(std::bind(&HttpData::handleWrite, this));
}

void HttpData::handleRead() {
    std::cout << "read " << std::this_thread::get_id() << "  " << loop_<< " " << fd_ << std::endl;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;
    rio_init(&rio, fd_);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);

    if (read_requesthdrs(&rio) == 1) {
        printf("read header not finish\n");
        return;
    }
    channel_->setEvent(CHANNEL_READ|CHANNEL_WRITE);
    loop_->updateChannel(channel_.get());
}

void HttpData::handleWrite() {
    std::cout << "handle write" << std::endl;
    struct stat sbuf;
    std::string filename = "./home.html";
    if (stat(filename.c_str(), &sbuf) < 0) {
        std::cout << "403" << std::endl;
        return;
    }
    serve_static(fd_, filename.c_str(), sbuf.st_size);
    channel_->setEvent(CHANNEL_READ);
    loop_->updateChannel(channel_.get());
}

void HttpData::newRequest() {
    channel_->setEvent(CHANNEL_READ);
    loop_->updateChannel(channel_.get());
}