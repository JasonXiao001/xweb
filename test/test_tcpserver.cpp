#include "../net/tcp_server.h"
#include "../net/tcp_connection.h"
#include "../net/event_loop.h"
#include <iostream>

using namespace std;

void onConn(const TcpConnectionPtr&conn) {
    conn->setTcpNoDelay(true);
    conn->send("hello\n");
}

void onMsg(const TcpConnectionPtr&conn, Buffer *buffer) {
    cout << conn->name() << " " << buffer->retrieveAllAsString() << endl;
}

void onWriteComplete(const TcpConnectionPtr&conn) {
    cout << conn->name() << " write done" << endl;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return -1;
    }
    int port = atoi(argv[1]);
    auto main_loop = std::make_shared<EventLoop>();
    TcpServer server(main_loop.get(), port);
    server.setConnectionCallback(onConn);
    server.setMessageCallback(onMsg);
    server.setWriteCompleteCallback(onWriteComplete);
    server.start();
    main_loop->loop();
    return 0;
}