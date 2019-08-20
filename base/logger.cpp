#include "logger.h"
#include <assert.h>
#include <iostream>
#include "async_logging.h"
#if defined(_WIN32)//Windows
#include "windows.h"
#else
#include <sys/time.h>
#endif

static Logger::LogLevel g_logLevel = Logger::E_TRACE;


void defaultOutput(const char* msg, int len)
{
    std::string str(msg, len);
    std::cout << str;
}

static Logger::OutputFunc g_output = defaultOutput;


const char* LogLevelName[Logger::NUM_LOG_LEVELS] =
{
    "TRACE",
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR",
    "FATAL",
};

inline LogStream& operator<<(LogStream& s, const SourceFile& v)
{
    s.append(v.data_, v.size_);
    return s;
}

Logger::Impl::Impl(LogLevel level, const SourceFile& file, int line) : 
    stream_(),
    level_(level),
    line_(line),
    basename_(file) {
    formatTime();
    // TODO log thread name
    // stream_ << T(CurrentThread::tidString(), CurrentThread::tidStringLength());
    stream_ << '|' << LogLevelName[level]
            << "|" << basename_ << ':' << line_ << ' ';
}

void Logger::Impl::formatTime() {
    char timeBufDst[64];
#ifdef _WIN32
	SYSTEMTIME st;
	GetLocalTime(&st);
	snprintf(timeBufDst, sizeof(timeBufDst), "%04d-%02d-%02d %02d:%02d:%02d.%03d", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
#else
	char timeBuf[64];
	time_t timeS = 0;
	int timeMs = 0;
	struct timeval tv{};
	if (0 == gettimeofday(&tv, nullptr)) {
		timeS = tv.tv_sec;
		timeMs = tv.tv_usec / 1000;
	} else {
		timeS = time(nullptr);
	}

	struct tm curTM{};
	if (nullptr == ::localtime_r(&timeS, &curTM))
		return;

	strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", &curTM);
	snprintf(timeBufDst, sizeof(timeBufDst), "%s.%03d", timeBuf, timeMs);
#endif
	stream_ << timeBufDst;
}

void Logger::Impl::finish()
{
    stream_ << '\n';
}

Logger::Logger(SourceFile file, int line)
  : impl_(E_INFO, file, line)
{
}

Logger::Logger(SourceFile file, int line, LogLevel level, const char* func)
  : impl_(level, file, line)
{
    impl_.stream_ << func << ' ';
}

Logger::Logger(SourceFile file, int line, LogLevel level)
  : impl_(level, file, line)
{
}

Logger::~Logger() {
    impl_.finish();
    const LogStream::Buffer& buf(stream().buffer());
    g_output(buf.data(), buf.length());
    if (impl_.level_ == E_FATAL) {
        abort();
    }
}

void Logger::setLogLevel(Logger::LogLevel level)
{
    g_logLevel = level;
}

void Logger::setOutput(OutputFunc out)
{
    g_output = out;
}

Logger::LogLevel Logger::logLevel() {
    return g_logLevel;
}