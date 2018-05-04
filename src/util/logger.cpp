#include "logger.hpp"

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
        return "TRACE";
    case Asuka::LogLevel::DEBUG:
        return "DEBUG";
    case Asuka::LogLevel::INFO:
        return "INFO";
    case Asuka::LogLevel::WARN:
        return "WARN";
    case Asuka::LogLevel::ERROR:
        return "ERROR";
    case Asuka::LogLevel::FATAL:
        return "FATAL";
    }

    return "UNKNOWN";
}

} // unamed namespace 

// default log level
#ifdef NDEBUG
LogLevel Logger::sLevel = LogLevel::INFO;
#else
LogLevel Logger::sLevel = LogLevel::DEBUG;
#endif // NDEBUG


void Logger::set_level(LogLevel lv)
{
    sLevel = lv;
}

LogLevel Logger::get_level()
{
    return sLevel;
}

Logger::Logger(LogLevel lv, const char* file, int line)
{
    
}

} // namespace Asuka