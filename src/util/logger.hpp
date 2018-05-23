// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_LOGGER_HPP
#define ASUKA_LOGGER_HPP

#include <cstdint>

#include "log_stream.hpp"

namespace Asuka
{

// reference muduo
// https://github.com/chenshuo/muduo/blob/master/muduo/base/Logging.h

enum class LogLevel
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


// for simplifying the source file name
struct SourceFile
{
    explicit SourceFile(const char* filename) : name(filename)
    {
        const char* slash = std::strrchr(name, '/');
        if (slash)
        {
            name = slash + 1;
        }
    }

    const char* name;
};

const char* errno_to_string_r(int savedErrno);

class Logger
{
public:
    using OutputFunc = void(*)(const char* msg, std::size_t len);
    using FlushFunc = void(*)();

public:
    static void set_level(LogLevel lv);
    static LogLevel get_level();

    static void set_output(OutputFunc func);
    static void set_flush(FlushFunc func);

    // yyyy-mm-dd hh:mm:ss.uuuuuu level thread_tag(id) message - xxx.cpp:line
    Logger(LogLevel lv, const char* file, int line);

    // yyyy-mm-dd hh:mm:ss.uuuuuu level thread_tag(id) func message - xxx.cpp:line
    Logger(LogLevel lv, const char* file, int line, const char* func);

    // yyyy-mm-dd hh:mm:ss.uuuuuu level thread_tag(id) errmsg message - xxx.cpp:line
    Logger(LogLevel lv, const char* file, int line, int savedError);


    ~Logger() noexcept;

    LogStream& get_stream();

    class Impl
    {
    public:
        Impl(LogLevel lv, const char* sf, int line);

        void finish();
    
        LogStream mStream;
        LogLevel mLevel;
        SourceFile mFilename;
        int mLine;
    };

private:
    void default_output(const char* msg, std::size_t len);
    void default_flush();

private:
    static LogLevel sLevel;
    static OutputFunc sOutFunc;
    static FlushFunc sFlushFunc;

    Impl mImpl;
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


#define LOG_SYSERROR \
    Logger(LogLevel::ERROR, __FILE__, __LINE__, errno).get_stream()
#define LOG_SYSFATAL \
    Logger(LogLevel::FATAL, __FILE__, __LINE__, errno).get_stream()

} // namespace Asuka

#endif // ASUKA_LOGGER_HPP