// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_LOGGER_HPP
#define ASUKA_LOGGER_HPP

#include <cstdint>

#include "log_stream.hpp"

namespace Asuka
{

enum class LogLevel : std::uint8_t
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
};

inline bool operator<=(LogLevel lhs, LogLevel rhs)
{
    return !(rhs < lhs);
}


class Logger
{
public:

public:
    static void set_level(LogLevel lv);
    static LogLevel get_level();

    Logger(LogLevel lv, const char* file, int line);
    Logger(LogLevel lv, const char* file, int line, const char* func);
    Logger(LogLevel lv, const char* file, int line, bool isAbort);
    ~Logger();

    LogStream& get_stream()
    {
        return mStream;
    }

private:
    static LogLevel sLevel;
    LogStream mStream;
};

#define LOG_TRACE \
    if (Logger::get_level() <= LogLevel::TRACE) \
        Logger(LogLevel::TRACE, __FILE__, __LINE__, __func__).get_stream()
        
#define LOG_DEBUG \
    if (Logger::get_level() <= LogLevel::DEBUG) \
        Logger(LogLevel::DEBUG, __FILE__, __LINE__, __func__).get_stream()

#define LOG_INFO \
    if (Logger::get_level() <= LogLevel::INFO) \
        Logger(LogLevel::INFO, __FILE__, __LINE__).get_stream()


#define LOG_WARN \
    Logger(LogLevel::WARN, __FILE__, __LINE__).get_stream()
#define LOG_ERROR \
    Logger(LogLevel::ERROR, __FILE__, __LINE__).get_stream()
#define LOG_FATAL \
    Logger(LogLevel::FATAL, __FILE__, __LINE__).get_stream()


#define LOG_SYSERR \
    Logger(LogLevel::ERROR, __FILE__, __LINE__, false).get_stream()
#define LOG_SYSFATAL \
    Logger(LogLevel::FATAL, __FILE__, __LINE__, true).get_stream()

} // namespace Asuka

#endif // ASUKA_LOGGER_HPP