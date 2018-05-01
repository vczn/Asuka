#include "time_stamp.hpp"

#include <inttypes.h>
#include <sys/time.h>

#include <cstdio>

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif // __STDC_FORMAT_MACROS

namespace Asuka
{


TimeStamp TimeStamp::now()
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    std::int64_t seconds = static_cast<std::int64_t>(tv.tv_sec);

    return TimeStamp{ seconds * kMicroPerSecond + tv.tv_usec };
}

TimeStamp TimeStamp::create_invalid_timestamp()
{
    return TimeStamp{};
}

TimeStamp TimeStamp::from_unix_time(time_t t)
{
    return from_unix_time(t, 0);
}

TimeStamp TimeStamp::from_unix_time(time_t t, std::int64_t restUs)
{
    std::int64_t us 
        = static_cast<std::int64_t>(t) * kMicroPerSecond + restUs;
    return TimeStamp{ us };
}

std::string TimeStamp::to_string() const
{
    char buf[32] = { 0 };
    std::int64_t seconds = mUs / kMicroPerSecond;
    std::int64_t us = mUs % kMicroPerSecond;
    snprintf(buf, sizeof(buf), 
        "%" PRId64 ".%06" PRId64 "", seconds, us);

    return std::string{ buf };
}

std::string TimeStamp::to_formatted_string(bool showUs) const
{
    char buf[32] = { 0 };
    time_t seconds = static_cast<time_t>(mUs / kMicroPerSecond);

    struct tm tmTime;
    if (!localtime_r(&seconds, &tmTime))
    {
        return "error time stamp";
    }

    if (showUs)
    {
        // yyyy-mm-dd hh:mm:ss.uuuuuu
        int us = static_cast<int>(mUs % kMicroPerSecond);
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d.%6d",
            tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday,
            tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec, us);
    }
    else
    {
        // yyyy-mm-dd hh:mm:ss
        snprintf(buf, sizeof(buf), "%4d-%02d-%02d %02d:%02d:%02d",
            tmTime.tm_year + 1900, tmTime.tm_mon + 1, tmTime.tm_mday,
            tmTime.tm_hour, tmTime.tm_min, tmTime.tm_sec);
    }

    return std::string{ buf };
}

bool TimeStamp::is_valid() const
{
    return mUs > 0;
}

std::int64_t TimeStamp::get_microseconds() const
{
    return mUs;
}

time_t TimeStamp::get_seconds() const
{
    return static_cast<time_t>(mUs / kMicroPerSecond);
}

TimeStamp& TimeStamp::operator+=(const Duration& dur)
{
    mUs += dur.to_microseconds();
    return *this;
}

TimeStamp& TimeStamp::operator-=(const Duration & dur)
{
    mUs -= dur.to_microseconds();
    return *this;
}

TimeStamp TimeStamp::operator+(const Duration& dur) const
{
    TimeStamp tmp(*this);
    tmp += dur;
    return tmp;
}

TimeStamp TimeStamp::operator-(const Duration & dur) const
{
    TimeStamp tmp(*this);
    tmp -= dur;
    return tmp;
}

Duration TimeStamp::operator-(const TimeStamp & rhs) const
{
    return Duration{ mUs - rhs.mUs };
}


} // mamespace Asuka
