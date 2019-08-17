#include "log_file.h"
#include "process_info.h"

#define SERVER (false)

LogFile::LogFile(const std::string& basename,
                off_t rollSize,
                bool threadSafe,
                int flushInterval,
                int checkEveryN) :
                basename_(basename),
                rollSize_(rollSize),
                threadSafe_(threadSafe),
                flushInterval_(flushInterval),
                checkEveryN_(checkEveryN) {
    rollFile();
}

bool LogFile::rollFile() {
    time_t now = 0;
    std::string filename = getLogFileName(basename_, now);
    time_t start = now / kRollPerSeconds_ * kRollPerSeconds_;

    if (now > lastRoll_) {
        lastRoll_ = now;
        lastFlush_ = now;
        startOfPeriod_ = start;
        file_.reset(new AppendFile(filename));
        return true;
    }
    return false;
}

std::string LogFile::getLogFileName(const std::string& basename, time_t &now) {
    std::string filename;
    filename.reserve(basename.size() + 64);
    filename = basename;

    char timebuf[32];
    struct tm tm;
    now = time(NULL);
#ifdef _WIN32
    auto ret = gmtime(&now);
    tm = *ret;
#else
    gmtime_r(&now, &tm);
#endif
    strftime(timebuf, sizeof timebuf, ".%Y%m%d.", &tm);
    filename += timebuf;
#ifndef _WIN32
    filename += ProcessInfo::hostname();
#endif
    if (SERVER) {
        char pidbuf[32];
        snprintf(pidbuf, sizeof pidbuf, ".%d", ProcessInfo::pid());
        filename += pidbuf;
    }
    filename += ".log";

    return filename;
}

void LogFile::append_unlocked(const char* logline, int len) {
    file_->append(logline, len);
    if (file_->writtenBytes() > rollSize_) {
        rollFile();
    }
    else {
        ++count_;
        if (count_ >= checkEveryN_) {
            count_ = 0;
            time_t now = ::time(NULL);
            time_t thisPeriod_ = now / kRollPerSeconds_ * kRollPerSeconds_;
            if (thisPeriod_ != startOfPeriod_) {
                rollFile();
            }
            else if (now - lastFlush_ > flushInterval_) {
                lastFlush_ = now;
                file_->flush();
            }
        }
    }
}

void LogFile::append(const char* logline, int len) {
    if (!threadSafe_) {
        std::lock_guard<std::mutex> lck(mtx_);
        append_unlocked(logline, len);
    }
    else {
        append_unlocked(logline, len);
    }
}

void LogFile::flush()
{
    if (threadSafe_) {
        std::lock_guard<std::mutex> lck(mtx_);
        file_->flush();
    }
    else {
        file_->flush();
    }
}