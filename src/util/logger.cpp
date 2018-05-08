#include "logger.hpp"

#include <cstdlib>

#include "config.hpp"
#include "current_thread.hpp"
#include "time_stamp.hpp"

namespace Asuka
{

namespace 
{

const char* level_to_string(LogLevel lv)
{
    switch (lv)
    {
    case LogLevel::TRACE:
        return "TRACE ";
    case LogLevel::DEBUG:
        return "DEBUG ";
    case LogLevel::INFO:
        return "INFO  ";
    case LogLevel::WARN:
        return "WARN  ";
    case LogLevel::ERROR:
        return "ERROR ";
    case LogLevel::FATAL:
        return "FATAL ";
    }

    return "UNKNOWN";
}

// strerror_r for thread safe
const std::size_t kErrnoBufferSize = 512;
thread_local char tErrnoBuf[kErrnoBufferSize];
const char* errno_to_string_r(int savedErrno)
{
    return strerror_r(savedErrno, tErrnoBuf, sizeof(tErrnoBuf));
}

} // unamed namespace 

// default log level
#ifdef NDEBUG
LogLevel Logger::sLevel = LogLevel::INFO;
#else
LogLevel Logger::sLevel = LogLevel::DEBUG;
#endif // NDEBUG
Logger::OutputFunc Logger::sOutFunc = nullptr;
Logger::FlushFunc Logger::sFlushFunc = nullptr;


void Logger::set_level(LogLevel lv)
{
    sLevel = lv;
}

LogLevel Logger::get_level()
{
    return sLevel;
}

void Logger::set_output(OutputFunc func)
{
    sOutFunc = func;
}

void Logger::set_flush(FlushFunc func)
{
    sFlushFunc = func;
}

Logger::Logger(LogLevel lv, const char* file, int line) 
    : mImpl(lv, file, line)
{
}

Logger::Logger(LogLevel lv, const char* file, int line,
    const char* func) : mImpl(lv, file, line)
{
    get_stream() << func << ' ';
}

Logger::Logger(LogLevel lv, const char* file, int line,
    int savedErrno) : mImpl(lv, file, line)
{
    get_stream() << errno_to_string_r(savedErrno) << ' ';
}

Logger::~Logger()
{
    mImpl.finish();
    const LogStream::LBuffer& buf = get_stream().get_buffer();
    if (sOutFunc)
    {
        sOutFunc(buf.data(), buf.size());
    }
    else
    {
        default_output(buf.data(), buf.size());
    }
    
    if (mImpl.mLevel == LogLevel::FATAL)
    {
        if (sFlushFunc)
        {
            sFlushFunc();
        }
        else
        {
            default_flush();
        }
        
        exit(EXIT_FAILURE);
    }
}

LogStream& Logger::get_stream()
{
    return mImpl.mStream;
}

void Logger::default_output(const char* msg, std::size_t len)
{
    std::size_t n = ::fwrite(msg, 1, len, stdout);  // thread safe
    if (n != len)
    {
        printf("log output error, content: %s\n", msg);
    }
}

void Logger::default_flush()
{
    std::fflush(stdout);
}

// TODO
// thread_id => thread_tag
Logger::Impl::Impl(LogLevel lv, const char* sf, int line)
    : mLevel(lv), mFilename(sf), mLine(line)
{
    mStream << TimeStamp::now().to_formatted_string() << ' '
        << level_to_string(lv) << ' '
        << current_thread_id_to_string() << ' ';
}

void Logger::Impl::finish()
{
    mStream << " - " << mFilename.name << ':' << mLine << '\n';
}

} // namespace Asuka