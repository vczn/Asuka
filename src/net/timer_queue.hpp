#pragma once
#ifndef ASUKA_TIMER_QUEUE_HPP
#define ASUKA_TIMER_QUEUE_HPP

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "../util/noncopyable.hpp"
#include "../util/time_stamp.hpp"
#include "callback.hpp"
#include "channel.hpp"


namespace Asuka
{

namespace Net
{

class EventLoop;
class Timer;
class TimerId;

class TimerQueue : Noncopyable
{
public:
    using Entry = std::pair<TimeStamp, std::unique_ptr<Timer>>;

    // to avoid the same key and order by timestamp
    // <<time, sequence>, timer>
    using TimerList = std::map<std::pair<TimeStamp, std::uint64_t>,
        std::unique_ptr<Timer>>;

    using TimerIdSet = std::set<TimerId>;

public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    TimerId add_timer(TimerCallback cb, TimeStamp when, double interval);
    void cancel(const TimerId& timerid);

private:
    void add_timer_in_loop(std::unique_ptr<Timer> timer);
    void cancel_in_loop(const TimerId& timerid);

    // call when timerfd alarms
    void handle_read();

    // remove expired timers
    std::vector<Entry> get_expired(TimeStamp now);

    void reset(std::vector<Entry>& expired, TimeStamp now);
    bool insert(std::unique_ptr<Timer> timer);

private:
    EventLoop* mLoop;
    const int mTimerFd;
    Channel mTimerChannel;

    // store timers
    TimerList mTimers;

    // for canceling
    TimerIdSet mActiveTimers;
    TimerIdSet mCancelTimers;
    bool mIsCallingExpiredTimers;  // is calling handle_read()
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_TIMER_QUEUE_HPP