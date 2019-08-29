#include "../net/acceptor.h"
#include "../net/event_loop.h"
#include "../net/inet_address.h"
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

void newConn(int sockfd, const InetAddress&) {
    std::string str = "hello";
    write(sockfd, str.c_str(), str.length());
    close(sockfd);
}

int main() {
    EventLoop loop;
    Acceptor ac(&loop, 5000);
    ac.listen();
    ac.setNewConnectionCallback(newConn);
    loop.loop();
    return 0;
}