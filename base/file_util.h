#pragma once
#include <string>
#include <sys/types.h>

class AppendFile {
public:
    explicit AppendFile(const std::string &path);
    AppendFile(const AppendFile&) = delete;
    ~AppendFile();
    // write file
    void append(const char *line, const size_t len);
    void flush();
    off_t writtenBytes() const { return writtenBytes_; }
private:
    size_t write(const char *logline, size_t len);
    FILE* fp_;
    off_t writtenBytes_;
    char buffer_[64*1024];
};
