#include "async_logging.h"
#include <assert.h>
#include <chrono>
#include <iostream>
#include "log_file.h"

static auto sec = std::chrono::seconds(1);

AsyncLogging::AsyncLogging(const std::string& basename,
                off_t rollSize,
                int flushInterval) :
                flushInterval_(flushInterval),
                running_(false),
                basename_(basename),
                rollSize_(rollSize),
                latch_(1),
                mutex_(),
                cond_(),
                console_log_(true),
                currentBuffer_(new Buffer),
                nextBuffer_(new Buffer),
                buffers_() {
    currentBuffer_->bzero();
    nextBuffer_->bzero();
    buffers_.reserve(16);
}

void AsyncLogging::append(const char* logline, int len)
{
    std::lock_guard<std::mutex> lck(mutex_);
    if (console_log_) {
        std::string str(logline, len);
        std::cout << str;
    }
    if (currentBuffer_->avail() > len) {
        currentBuffer_->append(logline, len);
    }
    else {
        buffers_.push_back(std::move(currentBuffer_));
        if (nextBuffer_) {
            currentBuffer_ = std::move(nextBuffer_);
        }
        else {
            currentBuffer_.reset(new Buffer); // Rarely happens
        }
        currentBuffer_->append(logline, len);
        cond_.notify_one();
    }
}



void AsyncLogging::threadFunc() {
    assert(running_ == true);
    LogFile output(basename_, rollSize_, false);
    BufferPtr newBuffer1(new Buffer);
    BufferPtr newBuffer2(new Buffer);
    newBuffer1->bzero();
    newBuffer2->bzero();

    BufferVector buffersToWrite;
    buffersToWrite.reserve(16);
    latch_.countDown();
    while (running_) {
        assert(newBuffer1 && newBuffer1->length() == 0);
        assert(newBuffer2 && newBuffer2->length() == 0);
        assert(buffersToWrite.empty());
        {
            std::unique_lock<std::mutex> lck(mutex_);
            if (buffers_.empty())  // unusual usage!
            {
                cond_.wait_for(lck, flushInterval_*sec);
            }
            buffers_.push_back(std::move(currentBuffer_));
            currentBuffer_ = std::move(newBuffer1);
            buffersToWrite.swap(buffers_);
            if (!nextBuffer_)
            {
                nextBuffer_ = std::move(newBuffer2);
            }
        }

        assert(!buffersToWrite.empty());

        if (buffersToWrite.size() > 25)
        {
            char buf[256];
            snprintf(buf, sizeof buf, "Dropped log messages %zd larger buffers\n",
                    buffersToWrite.size()-2);
            fputs(buf, stderr);
            output.append(buf, static_cast<int>(strlen(buf)));
            buffersToWrite.erase(buffersToWrite.begin()+2, buffersToWrite.end());
        }

        for (const auto& buffer : buffersToWrite)
        {
            // FIXME: use unbuffered stdio FILE ? or use ::writev ?
            output.append(buffer->data(), buffer->length());
        }

        if (buffersToWrite.size() > 2)
        {
            // drop non-bzero-ed buffers, avoid trashing
            buffersToWrite.resize(2);
        }

        if (!newBuffer1)
        {
            assert(!buffersToWrite.empty());
            newBuffer1 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer1->reset();
        }

        if (!newBuffer2)
        {
            assert(!buffersToWrite.empty());
            newBuffer2 = std::move(buffersToWrite.back());
            buffersToWrite.pop_back();
            newBuffer2->reset();
        }

        buffersToWrite.clear();
        output.flush();
    }
    output.flush();
}