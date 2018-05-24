// Part of Asuka net utility, public header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_EVENT_LOOP_HPP
#define ASUKA_EVENT_LOOP_HPP

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

#include "../util/any.hpp"
#include "../util/noncopyable.hpp"
#include "../util/time_stamp.hpp"
#include "callback.hpp"
#include "timer_id.hpp"

namespace Asuka
{

namespace Net
{

class Channel;
class PollerBase;
class TimerQueue;

class EventLoop : Noncopyable
{
public:
    using Function = std::function<void()>;

public:
    EventLoop();
    ~EventLoop();

    // must be called in same thread as creation of the object
    void loop();

    // it is thread safe that it is called through shared_ptr<EventLoop>
    void quit();

    // time stamp when poll returns, usually means data arrival
    TimeStamp poll_return_time() const;

    std::uint64_t iteration() const;

    // run callback function immediately in the loop thread
    // it wakes up the loop, and invoke the callback
    void run_in_loop(Function callback);

    // queue callback function in the loop thread
    // run after finish polling
    void queue_in_loop(Function callback);

    std::size_t queue_size() const;

    /// timers event

    // thread safe, call `callback` at `time`
    TimerId run_at(TimeStamp time, TimerCallback callback);

    // thread safe, call `callback` after `delay` seconds
    TimerId run_after(double delay, TimerCallback callback);

    // thread safe, call `callback` every `interval` seconds
    TimerId run_interval(double interval, TimerCallback callback);

    // thread safe
    void cancel_timer(const TimerId& timerid);

    // internal usage
    void wakeup();
    void update_channel(Channel& channel);
    void remove_channel(Channel& channel);
    bool has_channel(const Channel& channel) const;

    void assert_in_loop_thread() const;
    bool is_in_loop_thread() const;
    bool is_event_handing() const;

    void set_context(const Any& context);
    const Any& get_context() const;
    Any& get_context();

    static EventLoop* get_event_loop_current_thread();

private:
    void abort_not_in_loop_thread() const;
    void handle_read();  // wake up
    void pending_function();

    void print_active_channels() const; // DEBUG

private:
    std::atomic_bool mIsLoop;
    std::atomic_bool mIsQuit;
    std::atomic_bool mIsEventing;
    std::atomic_bool mIsCallingPendingFunction;

    std::uint64_t mIteration;

    const std::thread::id mThreadId; 
    TimeStamp mPollReturnTime;

    std::unique_ptr<PollerBase> mPoller;
    std::unique_ptr<TimerQueue> mTimerQueue;

    int mWakeupFd;
    std::unique_ptr<Channel> mWakeupChannel;

    Any mContext;

    std::vector<Channel*> mActiveChannels;
    Channel* mCurrentActiveChannel;

    mutable std::mutex mMutex;
    std::vector<Function> mPendingFunctions;  // Guarded by mMutex
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_EVENT_LOOP_HPP