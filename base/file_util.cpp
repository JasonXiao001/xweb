#include "file_util.h"
#include <assert.h>
#include <stdio.h>



AppendFile::AppendFile(const std::string &path) : 
    fp_(fopen(path.c_str(), "a")),
    writtenBytes_(0) {
#ifdef _WIN32
    setvbuf(fp_, buffer_, _IOFBF, sizeof buffer_);
#else
    setbuffer(fp_, buffer_, sizeof buffer_);
#endif
}

AppendFile::~AppendFile() {
    fclose(fp_);
}

void AppendFile::append(const char* line, const size_t len) {
    size_t n = this->write(line, len);
    size_t remain = len - n;
    while (remain > 0) {
        size_t x = this->write(line + n, remain);
        if (x == 0)
        {
            int err = ferror(fp_);
            if (err)
                fprintf(stderr, "AppendFile::append() failed !\n");
            break;
        }
        n += x;
        remain = len - n;
    }
    writtenBytes_ += len;
}

void AppendFile::flush() {
    fflush(fp_);
}

size_t AppendFile::write(const char* logline, size_t len) {
    return fwrite(logline, 1, len, fp_);
}