#pragma once

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <sys/types.h>
#include "countdown_latch.h"
#include "log_stream.h"

class AsyncLogging {
public:
    AsyncLogging(const std::string& basename,
                off_t rollSize,
                int flushInterval = 3);
    ~AsyncLogging() {
        if (running_) {
            stop();
        }
    }
    AsyncLogging(const AsyncLogging&) = delete;
    void append(const char* logline, int len);
    void start() {
        running_ = true;
        thread_ = std::move(std::thread(std::bind(&AsyncLogging::threadFunc, this)));
        latch_.wait();
    }
    void stop() {
        running_ = false;
        cond_.notify_one();
        thread_.join();
    }
private:
    void threadFunc();
    typedef FixedBuffer<kLargeBuffer> Buffer;
    typedef std::vector<std::unique_ptr<Buffer>> BufferVector;
    typedef BufferVector::value_type BufferPtr;
    const std::string basename_;
    CountDownLatch latch_;
    std::mutex mutex_;
    const off_t rollSize_;
    std::condition_variable cond_;
    std::thread thread_;
    const int flushInterval_;
    std::atomic<bool> running_;
    BufferPtr currentBuffer_;
    BufferPtr nextBuffer_;
    BufferVector buffers_;
    bool console_log_;
};