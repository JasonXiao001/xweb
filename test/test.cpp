#include <iostream>
#include "event_loop.h"
#include "http_server.h"

using namespace std;

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return -1;
    }
    int port = atoi(argv[1]);
    auto main_loop = std::make_shared<EventLoop>();
    HttpServer server(main_loop, 1, port);
    server.start();
    main_loop->loop();
    return 0;
}