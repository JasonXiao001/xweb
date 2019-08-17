#include "../base/logger.h"
#include "../base/async_logging.h"
#include <thread>
#include <unistd.h>

AsyncLogging *logging = nullptr;
off_t kRollSize = 500*1000*1000;

void writelog(const char* msg, int len) {
    logging->append(msg, len);
}

void logworker() {
    for (int i = 0; i < 10; ++i) {
        LOG_DEBUG << i << ' ' << std::this_thread::get_id();
        usleep(100000);
    }
}

int main() {
    logging = new AsyncLogging("testlog", kRollSize);
    logging->start();
    Logger::setOutput(writelog);
    std::thread thr1(logworker);
    std::thread thr2(logworker);
    std::thread thr3(logworker);
    std::thread thr4(logworker);
    
    LOG_DEBUG << "start test";
    thr1.join();thr2.join();thr3.join();thr4.join();
    logging->stop();
    return 0;
}