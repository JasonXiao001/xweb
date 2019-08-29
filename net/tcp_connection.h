#pragma once
#include <string>
#include <memory>
#include "inet_address.h"
#include "callback.h"
#include "../base/StringPiece.h"
#include "buffer.h"

class EventLoop;
class InetAddress;
class Channel;
class Socket;

using std::string;

class TcpConnection :
    public std::enable_shared_from_this<TcpConnection> {
    enum StateE { kDisconnected, kConnecting, kConnected, kDisconnecting };

public:
    explicit TcpConnection(EventLoop* loop,
                const string& name,
                int sockfd,
                const InetAddress& peerAddr);
    ~TcpConnection();
    TcpConnection(const TcpConnection&) = delete;

    EventLoop* getLoop() const { return loop_; }
    const string& name() const { return name_; }
    const InetAddress& peerAddress() const { return peerAddr_; }
    bool connected() const { return state_ == kConnected; }
    bool disconnected() const { return state_ == kDisconnected; }
    bool getTcpInfo(struct tcp_info*) const;
    string getTcpInfoString() const;
    void send(const void* message, int len);
    void send(const StringPiece& message);
    void send(Buffer&& message);
    void send(Buffer* message);  // this one will swap data
    void shutdown();
    void setTcpNoDelay(bool on);
    void connectEstablished();
    void setConnectionCallback(const ConnectionCallback& cb)
    { connectionCallback_ = cb; }

    void setMessageCallback(const MessageCallback& cb)
    { messageCallback_ = cb; }

    void setWriteCompleteCallback(const WriteCompleteCallback& cb)
    { writeCompleteCallback_ = cb; }

    void setHighWaterMarkCallback(const HighWaterMarkCallback& cb)
    { highWaterMarkCallback_ = cb; }

    void setCloseCallback(const CloseCallback& cb)
    { closeCallback_ = cb; }

private:
    void handleRead();
    void handleWrite();
    void handleClose();
    void handleError();
    void setState(StateE s) { state_ = s; }
    void sendInLoop(const StringPiece& message);
    void sendInLoop(const void* message, size_t len);
    string stateToString() const;
    void shutdownInLoop();

private:
    EventLoop *loop_;
    string name_;
    int sockfd_;
    const InetAddress peerAddr_;
    std::unique_ptr<Channel> channel_;
    std::unique_ptr<Socket> socket_;
    size_t highWaterMark_;
    StateE state_;
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;
    Buffer inputBuffer_;
    Buffer outputBuffer_; // FIXME: use list<Buffer> as output buffer.
};