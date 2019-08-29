#include "socket.h"
#include <sys/socket.h>
#include <sys/types.h> 
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h> 
#include "../base/logger.h"
#include "inet_address.h"


Socket::~Socket() {
    if (close(sockfd_) < 0) {
        LOG_ERROR << "sockets::close";
    }
}

bool Socket::getTcpInfo(struct tcp_info* tcpi) const {
    // socklen_t len = sizeof(*tcpi);
    // memset(tcpi, 0, len);
    // return getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
    return false;
}

int Socket::getSocketError() {
    // int optval;
    // socklen_t optlen = static_cast<socklen_t>(sizeof optval);
    // if (getsockopt(sockfd_, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    //     return errno;
    // }
    // else {
    //     return optval;
    // }
    return 0;
}

void Socket::bindAddress(const InetAddress& addr) {
    int ret = bind(sockfd_, addr.getSockAddr(), addr.sizeOfAddr());
    if (ret < 0) {
        LOG_FATAL << "bind " << ret;
    }
}

void Socket::listen() {
    int ret = ::listen(sockfd_, SOMAXCONN);
    if (ret < 0) {
        LOG_FATAL << "sockets::listenOrDie";
    }
}

int Socket::accept(InetAddress &peeraddr) {
    struct sockaddr_in addr;
    size_t addrlen = sizeof(addr);
    int connfd = ::accept(sockfd_, (struct sockaddr *)&addr, (socklen_t *)&addrlen);
    if (connfd >= 0) {
        peeraddr.setAddr(addr);
    }
    else {
        LOG_ERROR << "accept error " << connfd;
    }
    return connfd;
}

void Socket::shutdownWrite() {
    if (::shutdown(sockfd_, SHUT_WR) < 0) {
        LOG_ERROR << "sockets::shutdownWrite";
    }
}

void Socket::setTcpNoDelay(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::setReuseAddr(bool on)
{
  int optval = on ? 1 : 0;
  ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR,
               &optval, static_cast<socklen_t>(sizeof optval));
  // FIXME CHECK
}

void Socket::setReusePort(bool on)
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT,
                            &optval, static_cast<socklen_t>(sizeof optval));
    if (ret < 0 && on)
    {
        LOG_ERROR << "SO_REUSEPORT failed.";
    }
#else
    if (on)
    {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on)
{
    int optval = on ? 1 : 0;
    ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE,
                &optval, static_cast<socklen_t>(sizeof optval));
    // FIXME CHECK
}