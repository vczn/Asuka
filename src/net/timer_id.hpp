// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_TIMER_ID_HPP
#define ASUKA_TIMER_ID_HPP

#include "timer.hpp"

namespace Asuka
{

namespace Net
{

// for canceling the timer
class TimerId 
{
public:
    TimerId() 
        : mTimer(nullptr), 
          mSequence()
    {
    }

    TimerId(Timer* timer, std::uint64_t seq)
        : mTimer(timer), 
          mSequence(seq)
    {
    }

    TimerId(const TimerId& rhs)
        : mTimer(rhs.mTimer),
          mSequence(rhs.mSequence)
    {
    }

    TimerId& operator=(const TimerId& rhs)
    {
        if (this != &rhs)
        {
            TimerId tmp{ rhs };

            std::swap(mTimer, tmp.mTimer);
            std::swap(mSequence, tmp.mSequence);
        }

        return *this;
    }

    Timer* get_timer() const
    {
        return mTimer;
    }

    std::uint64_t get_sequence() const
    {
        return mSequence;
    }

private:
    Timer* mTimer;
    std::uint64_t mSequence;
};

inline bool operator<(const TimerId& lhs, const TimerId& rhs)
{
    return lhs.get_timer() < rhs.get_timer();
}

} // namespace Net

} // namespace Asuka

#endif // ASUKA_TIMER_ID_HPP