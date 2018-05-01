// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_UTIL_HPP
#define ASUKA_UTIL_HPP

#include <unistd.h>

#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "cxx_version.hpp"

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

inline void err_quit(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    err_impl(false, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

inline void err_sys(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    err_impl(true, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}

inline void err_ret(const char* fmt, ...)
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


// close file descriptor
inline void close_fd(int fd)
{
    if (::close(fd) < 0)
    {
        err_sysret("close error");
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

inline std::vector<std::string> split(const std::string& str, char delim)
{
    std::vector<std::string> result;
    split(str, delim, std::back_inserter(result));
    return result;
}

} // namespace Asuka

#endif // ASUKA_UTIL_HPP
