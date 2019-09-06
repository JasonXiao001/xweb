#include <string>
#ifndef __APPLE__
#include <strings.h>
#endif
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include "../base/rio.h"

#define SERVER_PORT 5000
#define MAXLINE 1024

using namespace std;

void str_cli(int sockfd) {
    char sendline[MAXLINE], recvline[MAXLINE];
    while(true) {
        ssize_t ret = ::read(sockfd, (void*)recvline, MAXLINE);
        cout << ret << endl;
        cout << recvline << endl;
        ::write(sockfd, recvline, ret);
    }
}

int main(int argc, char **argv) {
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
    connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));
    str_cli(sockfd);
    return 0;
}
