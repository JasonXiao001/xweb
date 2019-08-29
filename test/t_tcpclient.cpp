#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/select.h>
#include "../base/logger.h"

#define SERVER_PORT 5000
#define MAXLINE 1024

using namespace std;


int main() {
    fd_set rfds;
    struct timeval tv;
    int retval;
    FD_ZERO(&rfds);
    FD_SET(0, &rfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int count = 0;
    while(true) {
        retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);
        if (retval == 0) {
            if (count > 100) {
                continue;
            }
            ++count;
            int sockfd;
            struct sockaddr_in servaddr;
            sockfd = socket(AF_INET, SOCK_STREAM, 0);
            bzero(&servaddr, sizeof(servaddr));
            servaddr.sin_family = AF_INET;
            servaddr.sin_port = htons(SERVER_PORT);
            inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
            int ret = connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr));
            if (ret == 0) {
                LOG_INFO << "connect succeed, fd:" << sockfd;
                FD_SET(sockfd, &rfds);
            }else {
                LOG_ERROR << "connect error " << ret;
            }
        }else if (retval == -1) {
            LOG_FATAL << "select error";
        }else {
            for (int i = 0; i < FD_SETSIZE; ++i) {
                if (FD_ISSET(i, &rfds)) {
                    char line[MAXLINE];
                    read(i, line, MAXLINE);
                    LOG_INFO << "read form " << i << "|" << line;
                }
            }
        }
    }
    return 0;
}