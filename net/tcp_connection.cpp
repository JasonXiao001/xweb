#include "tcp_connection.h"
#include "channel.h"
#include "event_loop.h"
#include "../base/logger.h"
#include "../base/rio.h"
#include "../base/StringPiece.h"
#include "socket.h"
#include <assert.h>

using namespace google;

TcpConnection::TcpConnection(EventLoop* loop,
                const std::string& name,
                int sockfd,
                const InetAddress& peerAddr) :
                loop_(loop),
                name_(name),
                sockfd_(sockfd),
                peerAddr_(peerAddr),
                socket_(new Socket(sockfd)),
                channel_(new Channel(sockfd)),
                highWaterMark_(64*1024*1024),
                state_(kConnecting) {
    LOG_DEBUG << "name:" <<  name_ << " at " << this << " fd:" << sockfd;
    channel_->setReadHandler(std::bind(&TcpConnection::handleRead, this));
    channel_->setWriteHandler(std::bind(&TcpConnection::handleWrite, this));

}

TcpConnection::~TcpConnection() {
    LOG_DEBUG << "TcpConnection::dtor[" <<  name_ << "] at " << this
                << " fd=" << channel_->fd()
                << " state=" << stateToString();
    assert(state_ == kDisconnected);
}

bool TcpConnection::getTcpInfo(struct tcp_info* tcpi) const {
    return socket_->getTcpInfo(tcpi);
}

void TcpConnection::setTcpNoDelay(bool on) {
    socket_->setTcpNoDelay(on);
}


void TcpConnection::send(const void* data, int len) {
    send(StringPiece(static_cast<const char*>(data), len));
}

void TcpConnection::send(const StringPiece& message) {
    if (state_ == kConnected) {
        if (loop_->isLoopThread()) {
            sendInLoop(message);
        } else {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, message.as_string()));
        }
    }
}

void TcpConnection::send(Buffer* buf)
{
    if (state_ == kConnected) {
        if (loop_->isLoopThread()) {
            sendInLoop(buf->peek(), buf->readableBytes());
            buf->retrieveAll();
        }
        else
        {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf->retrieveAllAsString()));
        }
    }
}

void TcpConnection::send(Buffer &&buf)
{
    if (state_ == kConnected) {
        if (loop_->isLoopThread()) {
            sendInLoop(buf.peek(), buf.readableBytes());
            buf.retrieveAll();
        }
        else
        {
            void (TcpConnection::*fp)(const StringPiece& message) = &TcpConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp, this, buf.retrieveAllAsString()));
        }
    }
}

void TcpConnection::sendInLoop(const StringPiece& message) {
    sendInLoop(message.data(), message.size());
}

void TcpConnection::sendInLoop(const void* data, size_t len) {
    assert(loop_->isLoopThread());
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;
    if (state_ == kDisconnected) {
        LOG_WARN << "disconnected, give up writing";
        return;
    }
    // if nothing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = write(channel_->fd(), data, len);
        if (nwrote >= 0) {
            remaining = len - nwrote;
            if (remaining == 0 && writeCompleteCallback_) {
                loop_->runInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
            }
        }
        else {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_ERROR << "TcpConnection::sendInLoop";
                if (errno == EPIPE || errno == ECONNRESET) {
                    faultError = true;
                }
            }
        }
    }
    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_
            && oldLen < highWaterMark_
            && highWaterMarkCallback_) {
            loop_->runInLoop(std::bind(highWaterMarkCallback_, shared_from_this(), oldLen + remaining));
        }
        outputBuffer_.append(static_cast<const char*>(data)+nwrote, remaining);
        if (!channel_->isWriting()) {
            channel_->setEvent(channel_->events() | CHANNEL_WRITE);
            loop_->updateChannel(channel_.get());
        }
    }
}



void TcpConnection::connectEstablished() {
    assert(state_ == kConnecting);
    setState(kConnected);
    channel_->setEvent(CHANNEL_READ);
    loop_->updateChannel(channel_.get());
    connectionCallback_(shared_from_this());

}

void TcpConnection::shutdown() {
    if (state_ == kConnected) {
        setState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}


void TcpConnection::handleRead() {
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_);
    }
    else if (n == 0) {
        // TODO detect EOF from poller return
        handleClose();
    }
    else {
        errno = savedErrno;
        LOG_ERROR << "read ret " << n;
        handleError();
    }
}

void TcpConnection::handleWrite() {
    assert(loop_->isLoopThread());
    if (channel_->isWriting()) {
        ssize_t n = write(channel_->fd(), outputBuffer_.peek(), outputBuffer_.readableBytes());
        if (n > 0) {
            outputBuffer_.retrieve(n);
            if (outputBuffer_.readableBytes() == 0) {
                channel_->setEvent(channel_->events() & !CHANNEL_WRITE);
                loop_->updateChannel(channel_.get());
                if (writeCompleteCallback_) {
                    loop_->runInLoop(std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == kDisconnecting) {
                    shutdownInLoop();
                }
            }
        }
        else {
            LOG_ERROR << "write ret " << n;
        }
    }
    else {
        LOG_TRACE << "Connection fd = " << channel_->fd()
                << " is down, no more writing";
    }
}


void TcpConnection::handleClose() {
    assert(loop_->isLoopThread());
    LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
    assert(state_ == kConnected || state_ == kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(kDisconnected);
    loop_->removeChannel(channel_.get());
    closeCallback_(shared_from_this());
}

void TcpConnection::handleError() {
    int err = socket_->getSocketError();
    LOG_ERROR << "TcpConnection::handleError [" << name_
                << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}

string TcpConnection::stateToString() const
{
    switch (state_)
    {
        case kDisconnected:
        return "kDisconnected";
        case kConnecting:
        return "kConnecting";
        case kConnected:
        return "kConnected";
        case kDisconnecting:
        return "kDisconnecting";
        default:
        return "unknown state";
    }
}


void TcpConnection::shutdownInLoop() {
    assert(loop_->isLoopThread());

    if (!channel_->isWriting()) {
        // we are not writing
        socket_->shutdownWrite();
    }
}