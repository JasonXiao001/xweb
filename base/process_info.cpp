#include "process_info.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

std::string ProcessInfo::hostname() {
#ifdef _WIN32
    return "";
#else
    // HOST_NAME_MAX 64
    // _POSIX_HOST_NAME_MAX 255
    char buf[256];
    if (::gethostname(buf, sizeof buf) == 0) {
        buf[sizeof(buf)-1] = '\0';
        return buf;
    }
    else {
        return "unknownhost";
    }
#endif
}


#if defined(_WIN32)
DWORD ProcessInfo::pid() {
    return GetCurrentProcessId();
}
#else
pid_t ProcessInfo::pid() {
    return ::getpid();
}
#endif


