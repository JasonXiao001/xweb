#include "../net/event_loop.h"
#include <iostream>
#include <functional>
#include <unistd.h>

using namespace std;

EventLoop *loop = nullptr;
int id = 0;

void work() {
    cout << "do work" << endl;
}

void trigger() {
    sleep(5);
    cout << "cancel " << id <<endl;
    loop->cancel(id);
}

int main() {
    
    loop = new EventLoop();
    id = loop->runEvery(1000, work);
    std::thread thr(trigger);
    thr.detach();
    loop->loop();
    return 0;
}