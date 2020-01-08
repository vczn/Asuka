#pragma once
#ifndef ASUKA_DURATION_HPP
#define ASUKA_DURATION_HPP

#include <sys/time.h>

#include <cstdint>


namespace Asuka
{

// int64_t represent microseconds
class Duration
{
public:
    static const std::int64_t kMillisecond = 1000L;
    static const std::int64_t kSecond = 1000 * kMillisecond;
    static const std::int64_t kMinute = 60 * kSecond;
    static const std::int64_t kHour = 60 * kMinute;

public:
    Duration() = default;
    explicit Duration(const struct timeval& t)
        : mUs(t.tv_sec * kSecond + t.tv_usec)
    {
    }

    explicit Duration(std::int64_t us)
        : mUs(us)
    {
    }

    explicit Duration(double us)
        : mUs(static_cast<std::int64_t>(us))
    {
    }


    // return an integer microseconds count
    std::int64_t to_microseconds() const
    {
        return mUs;
    }

    double to_milliseconds() const
    {
        return static_cast<double>(mUs)
            / static_cast<double>(kMillisecond);
    }

    double to_seconds() const
    {
        return static_cast<double>(mUs) 
            / static_cast<double>(kSecond);
    }

    
    double to_minutes() const
    {
        return static_cast<double>(mUs)
            / static_cast<double>(kMinute);
    }

    double to_hours() const
    {
        return static_cast<double>(mUs)
            / static_cast<double>(kHour);
    }

    struct timeval to_timeval() const
    {
        struct timeval result;
        result.tv_sec = static_cast<time_t>(mUs / kSecond);
        result.tv_usec = static_cast<suseconds_t>(mUs % kSecond);

        return result;
    }

    bool is_zero() const
    {
        return mUs == 0;
    }

    static constexpr Duration zero()
    {
        return Duration{};
    }

    Duration& operator+=(const Duration& rhs)
    {
        mUs += rhs.mUs;
        return *this;
    }

    Duration& operator-=(const Duration& rhs)
    {
        mUs -= rhs.mUs;
        return *this;
    }

    Duration& operator*=(std::int64_t us)
    {
        mUs *= us;
        return *this;
    }

    Duration& operator/=(std::int64_t us)
    {
        mUs /= us;
        return *this;
    }
private:
    std::int64_t mUs;
};

inline bool operator==(Duration lhs, Duration rhs)
{
    return lhs.to_microseconds() == rhs.to_microseconds();
}

inline bool operator!=(Duration lhs, Duration rhs)
{
    return !(lhs == rhs);
}

inline bool operator<(Duration lhs, Duration rhs)
{
    return lhs.to_microseconds() < rhs.to_microseconds();
}

inline bool operator>(Duration lhs, Duration rhs)
{
    return rhs < lhs;
}

inline bool operator<=(Duration lhs, Duration rhs)
{
    return !(rhs < lhs);
}

inline bool operator>=(Duration lhs, Duration rhs)
{
    return !(lhs < rhs);
}

inline Duration operator+(Duration lhs, Duration rhs)
{
    return Duration{ lhs.to_microseconds() + rhs.to_microseconds() };
}

inline Duration operator-(Duration lhs, Duration rhs)
{
    return Duration{ lhs.to_microseconds() - rhs.to_microseconds() };
}

} // namespace Asuka

#endif // ASUKA_DURATION_HPP