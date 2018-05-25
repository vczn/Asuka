#include "event_loop.hpp"

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include <functional>
#include <thread>

#include "../util/logger.hpp"
#include "poller_base.hpp"
#include "timer_queue.hpp"

namespace Asuka
{

namespace Net
{

namespace 
{

thread_local EventLoop* tEventLoopInThisThread = nullptr;
const int kPollTimeoutMs = 10000;   // 10s


// for IPC, wakeup fd
int create_event_fd()
{
    // eventfd is similar to the pipe, but the eventfd only need a fd
    // In linux up to the version 2.6.26
    // eventfd flags argument is unused and must be specified as zero
    int evtfd = ::eventfd(0, 0);
    if (evtfd < 0)
    {
        LOG_SYSFATAL << "eventfd error";
    }

    return evtfd;
}

struct IgnoreSignalPipe
{
    IgnoreSignalPipe()
    {
        ::signal(SIGPIPE, SIG_IGN);
    }
};

IgnoreSignalPipe gInitObj;

} // unamed namespace

EventLoop::EventLoop() 
    : mIsLoop(false),
      mIsQuit(false),
      mIsEventing(false),
      mIsCallingPendingFunction(false),
      mIteration(0),
      mThreadId(std::this_thread::get_id()),
      mPollReturnTime(),
      mPoller(PollerBase::create_default_poller(this)),
      mTimerQueue(new TimerQueue{this}),
      mWakeupFd(create_event_fd()),
      mWakeupChannel(new Channel{this, mWakeupFd}),
      mContext(),
      mCurrentActiveChannel(nullptr)
{
    LOG_DEBUG << "EventLoop is created";
    if (tEventLoopInThisThread)
    {
        LOG_FATAL << "current thread eventloop has existed";
    }
    else
    {
        tEventLoopInThisThread = this;
    }

    // always read the wakeup fd
    mWakeupChannel->set_read_callback(
        std::bind(&EventLoop::handle_read, this));
    mWakeupChannel->enable_read();
}

EventLoop::~EventLoop()
{
    LOG_DEBUG << "Eventloop is being destory";
    mWakeupChannel->disable_all();
    mWakeupChannel->remove();
    close_fd(mWakeupFd);
    tEventLoopInThisThread = nullptr;
}

void EventLoop::assert_in_loop_thread() const
{
    if (!is_in_loop_thread())
    {
        abort_not_in_loop_thread();
    }
}

bool EventLoop::is_in_loop_thread() const
{
    return mThreadId == std::this_thread::get_id();
}

bool EventLoop::is_event_handing() const
{
    return mIsEventing;
}

void EventLoop::set_context(const Any& context)
{
    mContext = context;
}

const Any& EventLoop::get_context() const
{
    return mContext;
}

Any& EventLoop::get_context()
{
    return mContext;
}

EventLoop* EventLoop::get_event_loop_current_thread()
{
    return tEventLoopInThisThread;
}

void EventLoop::abort_not_in_loop_thread() const
{
    LOG_FATAL << "eventloop abort not in the loop thread";
}

void EventLoop::handle_read()
{
    // read wake up fd
    std::uint64_t one = 1;
    ssize_t n = ::read(mWakeupFd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::handle_read reads " << n << " bytes";
    }
}

void EventLoop::pending_function()
{
    std::vector<Function> functions;
    mIsCallingPendingFunction = true;

    {
        std::lock_guard<std::mutex> lock{ mMutex };
        functions.swap(mPendingFunctions);
    }

    for (auto& function : functions)
    {
        function();
    }

    mIsCallingPendingFunction = false;
}

void EventLoop::print_active_channels() const
{
    for (const Channel* channel : mActiveChannels)
    {
        LOG_TRACE << '{' << channel->revents_to_string() << '}';
    }
}

void EventLoop::loop()
{
    assert(!mIsLoop);
    assert_in_loop_thread();

    mIsLoop = true;
    mIsQuit = false;    // FIXME if someone calls quit() before loop
    LOG_TRACE << "EventLoop " << this << "start looping";

    while (!mIsQuit)
    {
        mActiveChannels.clear();
        mPollReturnTime = mPoller->poll(kPollTimeoutMs, mActiveChannels);
        ++mIteration;
        if (Logger::get_level() <= LogLevel::TRACE)
        {
            print_active_channels();
        }

        mIsEventing = true;
        for (Channel* channel : mActiveChannels)
        {
            mCurrentActiveChannel = channel;
            mCurrentActiveChannel->handle_event(mPollReturnTime);
        }

        mCurrentActiveChannel = nullptr;
        mIsEventing = false;
        pending_function();
    }

    LOG_TRACE << "EventLoop " << this << "stop looping";
    mIsLoop = false;
}

void EventLoop::quit()
{
    mIsQuit = true;
    if (!is_in_loop_thread())
    {
        wakeup();
    }
}

TimeStamp EventLoop::poll_return_time() const
{
    return mPollReturnTime;
}

std::uint64_t EventLoop::iteration() const
{
    return mIteration;
}

void EventLoop::run_in_loop(Function callback)
{
    if (is_in_loop_thread())
    {
        callback();
    }
    else
    {
        queue_in_loop(std::move(callback));
    }
}

void EventLoop::queue_in_loop(Function callback)
{
    {
        std::lock_guard<std::mutex> lock{ mMutex };
        mPendingFunctions.push_back(std::move(callback));
    }

    if (!is_in_loop_thread() || mIsCallingPendingFunction)
    {
        wakeup();
    }
}

std::size_t EventLoop::queue_size() const
{
    std::lock_guard<std::mutex> lock{ mMutex };
    return mPendingFunctions.size();
}

TimerId EventLoop::run_at(TimeStamp time, TimerCallback callback)
{
    return mTimerQueue->add_timer(std::move(callback), time, 0.0);
}

TimerId EventLoop::run_after(double delay, TimerCallback callback)
{
    TimeStamp time = TimeStamp::now() + Duration{ delay };
    return run_at(time, std::move(callback));
}

TimerId EventLoop::run_interval(double interval, TimerCallback callback)
{
    TimeStamp time = TimeStamp::now() + Duration{ interval };
    return mTimerQueue->add_timer(std::move(callback), time, interval);
}

void EventLoop::cancel_timer(const TimerId& timerid)
{
    mTimerQueue->cancel(timerid);
}

void EventLoop::wakeup()
{
    std::uint64_t one = 1;
    ssize_t n = ::write(mWakeupFd, &one, sizeof(one));
    if (n != sizeof(one))
    {
        LOG_ERROR << "EventLoop::wakeup writes " << n << " bytes";
    }
}

void EventLoop::update_channel(Channel& channel)
{
    assert(channel.get_owner_loop() == this);
    assert_in_loop_thread();
    mPoller->update_channel(channel);
}

void EventLoop::remove_channel(Channel& channel)
{
    assert(channel.get_owner_loop() == this);
    assert_in_loop_thread();
    if (mIsEventing)
    {
        assert(mCurrentActiveChannel == &channel ||
            std::find(mActiveChannels.begin(), mActiveChannels.end(), &channel)
                == mActiveChannels.end());
    }
    mPoller->remove_channel(channel);
}

bool EventLoop::has_channel(const Channel& channel) const
{
    assert(channel.get_owner_loop() == this);
    assert_in_loop_thread();
    return mPoller->has_channel(channel);
}

} // namespace Net

} // namespace Asuka