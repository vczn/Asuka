// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_TIME_STAMP_HPP
#define ASUKA_TIME_STAMP_HPP

#include <cstdint>
#include <string>

#include "duration.hpp"

namespace Asuka
{

// reference muduo
// https://github.com/chenshuo/muduo/blob/cpp11/muduo/base/Timestamp.h

// int64_t represend a timestamp since epoch, in microseconds resolution
class TimeStamp
{
public:
    static const std::int64_t kMicroPerSecond = 1000000L;
public:
    constexpr TimeStamp() : mUs(0)
    {
    }

    explicit TimeStamp(std::int64_t us) : mUs(us)
    {
    }

    explicit TimeStamp(const struct timeval& t)
        : mUs(t.tv_sec * kMicroPerSecond + t.tv_usec)
    {
    }

    TimeStamp(const TimeStamp& rhs) : mUs(rhs.mUs)
    {
    }

    void swap(TimeStamp& rhs) noexcept
    {
        std::swap(mUs, rhs.mUs);
    }

    TimeStamp& operator=(const TimeStamp& rhs)
    {
        if (this != &rhs)
        {
            TimeStamp tmp{ rhs };
            swap(tmp);
        }
        return *this;
    }

    // return a current utc time
    static TimeStamp now();

    static TimeStamp create_invalid_timestamp();

    static TimeStamp from_unix_time(time_t t);
    static TimeStamp from_unix_time(time_t t, std::int64_t restUs);

    // ssssssss.uuuuuu
    std::string to_string() const;

    // to yyyy-mm-dd hh:mm:ss.uuuuuu local time
    std::string to_formatted_string(bool showUs = true) const;

    // > 0
    bool is_valid() const;

    std::int64_t get_microseconds() const;

    // since 00:00, Jan 1 1970 UTC, corresponding to POSIX time
    time_t get_seconds() const;

    TimeStamp& operator+=(const Duration& dur);
    TimeStamp& operator-=(const Duration& dur);

    TimeStamp operator+(const Duration& dur) const;
    TimeStamp operator-(const Duration& dur) const;
    Duration operator-(const TimeStamp& rhs) const;
private:
    std::int64_t mUs;
};

inline bool operator==(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get_microseconds() == rhs.get_microseconds();
}

inline bool operator!=(TimeStamp lhs, TimeStamp rhs)
{
    return !(lhs == rhs);
}

inline bool operator<(TimeStamp lhs, TimeStamp rhs)
{
    return lhs.get_microseconds() < rhs.get_microseconds();
}

inline bool operator>(TimeStamp lhs, TimeStamp rhs)
{
    return rhs < lhs;
}

inline bool operator<=(TimeStamp lhs, TimeStamp rhs)
{
    return !(rhs < lhs);
}

inline bool operator>=(TimeStamp lhs, TimeStamp rhs)
{
    return !(lhs < rhs);
}

} // namespace Asuka


#endif // ASUKA_TIME_STAMP_HPP