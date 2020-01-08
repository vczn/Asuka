#pragma once
#ifndef ASUKA_UTIL_HPP
#define ASUKA_UTIL_HPP

#include <string>
#include <vector>

#include "cxx_version.hpp"

namespace Asuka
{

void err_quit(const char* fmt, ...);
void err_sys(const char* fmt, ...);
void err_ret(const char* fmt, ...);
void err_sysret(const char* fmt, ...);


// close file descriptor
void close_fd(int fd, const char* str);
void close_fd(int fd);

std::vector<std::string> split(const std::string& str, char delim);


template <typename T>
inline T* check_not_nullptr(T* ptr)
{
    if (!ptr)
        err_quit("ptr is nullptr");
    return ptr;
}

} // namespace Asuka

#endif // ASUKA_UTIL_HPP