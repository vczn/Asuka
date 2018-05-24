#include "util.hpp"

#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "logger.hpp"

namespace Asuka
{

namespace 
{
// error handle function
// be used before logger initialization is complete
inline void err_impl(bool errnoflag, const char* fmt, va_list args)
{
    char buffer[128];
    std::vsprintf(buffer, fmt, args);

    if (errnoflag)
    {
        std::sprintf(buffer, ": %s", std::strerror(errno));
    }
    std::strcat(buffer, "\n");
    std::fflush(stdout);
    std::fputs(buffer, stderr);
    std::fflush(nullptr);
}

} // unamed namespace


void err_quit(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    err_impl(false, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void err_sys(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    err_impl(true, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

void err_ret(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    err_impl(false, fmt, args);
    va_end(args);
    return;
}

void err_sysret(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    err_impl(true, fmt, args);
    va_end(args);
    return;
}

void close_fd(int fd, const char* str)
{
    if (str == nullptr)
    {
        close_fd(fd);
        return;
    }

    if (::close(fd) == -1)
    {
        LOG_SYSERROR << str;
    }
}

void close_fd(int fd)
{
    if (::close(fd) == -1)
    {
        LOG_SYSERROR << "close fd error";
    }
}

namespace
{

template <typename Out>
inline void split(const std::string& str, char delim, Out result)
{
    std::stringstream ss;
    ss.str(str);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        *result = item;
    }
}

} // unamed namespace

std::vector<std::string> split(const std::string& str, char delim)
{
    std::vector<std::string> result;
    split(str, delim, std::back_inserter(result));
    return result;
}

} // namespace Asuka