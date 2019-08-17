#pragma once
#include "log_stream.h"


class SourceFile
{
public:
    template<int N>
    SourceFile(const char (&arr)[N])
      : data_(arr),
        size_(N-1)
    {
        const char* slash = strrchr(data_, '/'); // builtin function
        if (!slash) 
        {
             slash = strrchr(data_, '\\');
        }
        if (slash)
        {
            data_ = slash + 1;
            size_ -= static_cast<int>(data_ - arr);
        }
    }
explicit SourceFile(const char* filename)
    : data_(filename)
    {
        const char* slash = strrchr(filename, '/');
        if (!slash) 
        {
             slash = strrchr(data_, '\\');
        }
        if (slash) 
        {
            data_ = slash + 1;
        }
        size_ = static_cast<int>(strlen(data_));
    }

    const char* data_;
    int size_;
};

class Logger {
public:
    enum LogLevel
    {
        E_TRACE,
        E_DEBUG,
        E_INFO,
        E_WARN,
        E_ERROR,
        E_FATAL,
        NUM_LOG_LEVELS,
    };
    ~Logger();
    Logger(SourceFile file, int line);
    Logger(SourceFile file, int line, LogLevel level);
    Logger(SourceFile file, int line, LogLevel level, const char* func);
    static LogLevel logLevel();
    LogStream& stream() { return impl_.stream_; }
    typedef void (*OutputFunc)(const char* msg, int len);
    static void setOutput(OutputFunc);
    static void setLogLevel(Logger::LogLevel level); 

private:
    struct Impl
    {
        typedef Logger::LogLevel LogLevel;
        Impl(LogLevel level, const SourceFile& file, int line);
        void formatTime();
        void finish();

        LogStream stream_;
        LogLevel level_;
        int line_;
        SourceFile basename_;
    };
    Impl impl_;
};

#define LOG_TRACE if (Logger::logLevel() <= Logger::E_TRACE) \
    Logger(__FILE__, __LINE__,  Logger::E_TRACE, __func__).stream()
#define LOG_DEBUG if (Logger::logLevel() <= Logger::E_DEBUG) \
    Logger(__FILE__, __LINE__, Logger::E_DEBUG, __func__).stream()
#define LOG_INFO if (Logger::logLevel() <= Logger::E_INFO) \
    Logger(__FILE__, __LINE__).stream()
#define LOG_WARN Logger(__FILE__, __LINE__, Logger::E_WARN).stream()
#define LOG_ERROR Logger(__FILE__, __LINE__, Logger::E_ERROR).stream()
#define LOG_FATAL Logger(__FILE__, __LINE__, Logger::E_FATAL).stream()