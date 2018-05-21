#include "timer_queue.hpp"

#include <sys/timerfd.h>
#include <unistd.h>

#include <cassert>
#include <climits>
#include <cstring>

#include "../util/logger.hpp"
#include "channel.hpp"
#include "event_loop.hpp"
#include "timer.hpp"
#include "timer_id.hpp"

namespace Asuka
{

namespace Net
{

// helper function
namespace
{

int create_timerfd()
{
    // create a nonsettable monotonically increasing clock that 
    // delivers timer expiration notifications via a file descriptor
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC,
        TFD_NONBLOCK | TFD_CLOEXEC);
    if (timerfd < 0)
    {
        LOG_SYSERROR << "timerfd_create error";
    }

    return timerfd;
}

struct timespec get_time_from_now(TimeStamp later)
{
    std::uint64_t us = later.get_microseconds()
        - TimeStamp::now().get_microseconds();

    // to avoid overflowing
    assert(us < INT_MAX);

    if (us < 100)
    {
        us = 100;
    }

    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(us / 1000000);
    ts.tv_nsec = static_cast<long>((us % 1000000) * 1000);

    return ts;
}

void read_timerfd(int timerfd, TimeStamp now)
{
    std::uint64_t howmany;
    ssize_t n = ::read(timerfd, &howmany, sizeof(howmany));

    LOG_TRACE << "read " << howmany << " timer_fd";
    
    if (n != sizeof(howmany))
    {
        LOG_SYSERROR << "read timer fd error";
    }
}

void reset_timerfd(int timerfd, TimeStamp ts)
{
    // wake up the loop by timerfd_settime
    struct itimerspec oldValue, newValue;
    ::bzero(&oldValue, sizeof(oldValue));
    ::bzero(&newValue, sizeof(newValue));

    // initial expiration
    newValue.it_value = get_time_from_now(ts);

    if (::timerfd_settime(timerfd, 0, &newValue, &oldValue) == -1)
    {
        LOG_SYSERROR << "timerfd_settime error";
    }
}

} // unamed namespace


TimerQueue::TimerQueue(EventLoop* loop)
    : mLoop(loop), 
      mTimerFd(create_timerfd()),
      mTimerChannel(loop, mTimerFd),
      mTimers(),
      mActiveTimers(),
      mCancelTimers(),
      mIsCallingExpiredTimers(false)
{
    // always read the timerfd
    mTimerChannel.set_read_callback(
        std::bind(&TimerQueue::handle_read, this));

    mTimerChannel.enable_read();
}

TimerQueue::~TimerQueue()
{
    mTimerChannel.disable_all();
    mTimerChannel.remove();
    if (::close(mTimerFd) == -1)
    {
        LOG_SYSERROR << "close timerfd error";
    }
    // automatically destroy and release every timer in mTimers
}

TimerId TimerQueue::add_timer(TimerCallback cb, TimeStamp when, double interval)
{
#ifdef ASUKACXX14
    auto timer = std::make_unique<Timer>(std::move(cb), when, interval);
#else 
    std::unique_ptr<Timer> timer{ new Timer{std::move(cb), when, interval} };
#endif // CXX14
    TimerId timerid{ timer.get(), timer->get_sequence() };

    mLoop->run_in_loop(std::bind([this, &timer]
        () { this->add_timer_in_loop(std::move(timer)); }));

    return timerid;
}

void TimerQueue::cancel(const TimerId& timerid)
{
    mLoop->run_in_loop([this, &timerid]
        () { this->cancel_in_loop(timerid); });
}

void TimerQueue::add_timer_in_loop(std::unique_ptr<Timer> timer)
{
    mLoop->assert_in_loop_thread();
    
    bool earliestChanged = insert(std::move(timer));
    if (earliestChanged)
    {
        reset_timerfd(mTimerFd, timer->get_expiration());
    }
}

void TimerQueue::cancel_in_loop(const TimerId& timerid)
{
    mLoop->assert_in_loop_thread();
    assert(mTimers.size() == mActiveTimers.size());
    
    auto iter = mActiveTimers.find(timerid);
    if (iter != mActiveTimers.end())
    {
        // find it
        std::size_t n = mTimers.erase({ iter->get_timer()->get_expiration(), 
            iter->get_sequence() });
        assert(n == 1);
        (void)n;
        mActiveTimers.erase(iter);
    }
    else if (mIsCallingExpiredTimers)
    {
        mCancelTimers.insert(timerid);
    }
    assert(mTimers.size() == mActiveTimers.size());
}

void TimerQueue::handle_read()
{
    mLoop->assert_in_loop_thread();
    TimeStamp now = TimeStamp::now();

    read_timerfd(mTimerFd, now);

    std::vector<Entry> expireds = get_expired(now);

    mIsCallingExpiredTimers = true;
    mCancelTimers.clear();
    for (auto& e : expireds)
    {
        e.second->run();
    }
    mIsCallingExpiredTimers = false;

    reset(expireds, now);
}

std::vector<TimerQueue::Entry> TimerQueue::get_expired(TimeStamp now)
{
    assert(mTimers.size() == mActiveTimers.size());

    std::vector<Entry> expireds;
    std::pair<TimeStamp, std::uint64_t> key{ now, UINT64_MAX };

    auto end = mTimers.lower_bound(key);

    // move the expired timer to `expireds`
    for (auto iter = mTimers.begin(); iter != end; ++iter)
    {
        expireds.emplace_back(iter->first.first, std::move(iter->second));
    }

    // erase the expired timer from `mActiveTimers`
    for (const auto& entry : expireds)
    {
        TimerId timerid{ entry.second.get(), entry.second->get_sequence() };
        std::size_t n = mActiveTimers.erase(timerid);
        assert(n == 1);
        (void)n;
    }

    // earse the expired timers from `mTimers`
    mTimers.erase(mTimers.begin(), end);

    assert(mTimers.size() == mActiveTimers.size());

    return expireds;
}

void TimerQueue::reset(std::vector<Entry>& expired, TimeStamp now)
{
    for (auto& e : expired)
    {
        TimerId timerid{ e.second.get(), e.second->get_sequence() };
        if (e.second->is_repeat()
            && mCancelTimers.find(timerid) == mCancelTimers.end())
        {
            e.second->restart(now);
            insert(std::move(e.second));
        }
    }

    TimeStamp nextExpired;
    if (!mTimers.empty())
    {
        nextExpired = mTimers.begin()->second->get_expiration();
    }

    if (nextExpired.is_valid())
    {
        reset_timerfd(mTimerFd, nextExpired);
    }
}

bool TimerQueue::insert(std::unique_ptr<Timer> timer)
{
    mLoop->assert_in_loop_thread();
    assert(mTimers.size() == mActiveTimers.size());

    bool earliestChanged = false;
    TimeStamp timestamp = timer->get_expiration();
    if (mTimers.empty() || timestamp < mTimers.begin()->first.first)
    {
        earliestChanged = true;
    }

    // insert into `mActiveTimers`
    {
        auto p = mActiveTimers.insert(TimerId{ timer.get(), timer->get_sequence() });
        assert(p.second);
        (void)p;
    }
    
    // insert into `mTimers`
    {
        std::uint64_t seq = timer->get_sequence();
        auto p = mTimers.emplace(std::pair<TimeStamp, std::uint64_t>{ timestamp, seq }, 
            std::move(timer));
        assert(p.second);
        (void)p;
    }

    return earliestChanged;
}

} // namespace Net

} // namespace Asuka