#pragma once
#include "file_util.h"
#include <mutex>
#include <memory>
#include <string>

class LogFile {
public:
    LogFile(const std::string& basename,
            off_t rollSize,
            bool threadSafe,
            int flushInterval = 3,
            int checkEveryN = 1024);
    ~LogFile() = default;
    LogFile(const LogFile&) = delete;

    void append(const char* logline, int len);
    void flush();
    bool rollFile();
    
private:
    void append_unlocked(const char* logline, int len);
    std::string getLogFileName(const std::string& basename, time_t &now);

    const std::string basename_;
    const off_t rollSize_;
    const int checkEveryN_;
    const int flushInterval_;
    const bool threadSafe_;
    time_t startOfPeriod_;
    time_t lastRoll_;
    time_t lastFlush_;
    int count_;
    std::mutex mtx_;
    std::unique_ptr<AppendFile> file_;
    const static int kRollPerSeconds_ = 60*60*24;
};