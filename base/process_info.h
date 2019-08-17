#if defined(_WIN32)//Windows
#include <Windows.h>
#else
#include <unistd.h>
#endif
#include <string>


struct ProcessInfo {
    static std::string hostname();
#if defined(_WIN32)//Windows
    static DWORD pid();
#else
    static pid_t pid();
#endif
};


