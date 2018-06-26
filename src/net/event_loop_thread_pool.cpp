#include "event_loop_thread_pool.hpp"

#include <cassert>

#include "event_loop.hpp"
#include "event_loop_thread.hpp"

namespace Asuka
{

namespace Net
{

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop, std::string name)
    : mBaseLoop(baseLoop),
      mName(std::move(name)),
      mIsStarted(false),
      mNumThreads(0),
      mNext(0),
      mThreads(),
      mLoops()
{
}

EventLoopThreadPool::~EventLoopThreadPool()
{
}

void EventLoopThreadPool::set_thread_number(std::size_t num)
{
    // set number of threads before starting
    assert(!mIsStarted); 
    mNumThreads = num;
}

void EventLoopThreadPool::start(ThreadInitCallback cb)
{
    assert(!mIsStarted);
    mBaseLoop->assert_in_loop_thread();

    mIsStarted = true;

    for (std::size_t i = 0; i < mNumThreads; ++i)
    {
        // TODO: thread tag
        // TODO: use make_unique
        EventLoopThread* t = new EventLoopThread(cb);
        mThreads.push_back(std::unique_ptr<EventLoopThread>(t));
        mLoops.push_back(t->startLoop());
    }

    if (mNumThreads == 0 && cb)
    {
        cb(mBaseLoop);
    }
}

EventLoop* EventLoopThreadPool::get_next_loop()
{
    mBaseLoop->assert_in_loop_thread();
    assert(mIsStarted);

    EventLoop* loop = mBaseLoop;

    if (!mLoops.empty())
    {
        // round-robin
        loop = mLoops[mNext++];
        if (static_cast<std::size_t>(mNext) >= mLoops.size())
        {
            mNext = 0;
        }
    }

    return loop;
}

EventLoop* EventLoopThreadPool::get_loop_for_hash(std::size_t hashCode)
{
    mBaseLoop->assert_in_loop_thread();
    EventLoop* loop = mBaseLoop;

    if (!mLoops.empty())
    {
        loop = mLoops[hashCode % mLoops.size()];
    }

    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::get_all_loops()
{
    return mLoops;
}

bool EventLoopThreadPool::started() const
{
    return mIsStarted;
}

const std::string& EventLoopThreadPool::get_name() const
{
    return mName;
}

} // namespace Net

} // namespace Asuka

