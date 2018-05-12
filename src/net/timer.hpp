// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_TIMER_HPP
#define ASUKA_TIMER_HPP

#include <atomic>

#include "../util/duration.hpp"
#include "../util/noncopyable.hpp"
#include "../util/time_stamp.hpp"
#include "callback.hpp"

namespace Asuka
{

namespace Net
{

// Timer event callback
class Timer : Noncopyable
{
public:
    Timer(TimerCallback cb, TimeStamp when, double interval)
        : mCallback(std::move(cb)),
          mExpiration(when),
          mInterval(interval),
          mRepeat(interval > 0.0),
          mSequence(++sNumCreated)
    {
    }

    void run()
    {
        mCallback();
    }

    bool is_repeat() const noexcept
    {
        return mRepeat;
    }

    std::uint64_t get_sequence() const
    {
        return mSequence;
    }

    static std::uint64_t get_number_timers()
    {
        return sNumCreated;
    }

    void restart(TimeStamp now)
    {
        if (mRepeat)
        {
            mExpiration = now + mInterval;
        }
        else
        {
            mExpiration = TimeStamp::create_invalid_timestamp();
        }
    }
private:
    const TimerCallback mCallback;
    TimeStamp mExpiration;
    const Duration mInterval;
    const bool mRepeat;
    const std::uint64_t mSequence;

    static std::atomic<std::uint64_t> sNumCreated;
};

}

} // namespace Asuka

#endif // ASUKA_TIMER_HPP