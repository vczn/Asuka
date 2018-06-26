#include "event_loop_thread.hpp"

#include "event_loop.hpp"

namespace Asuka
{

namespace Net
{
    
EventLoopThread::EventLoopThread(ThreadInitCallback cb)
    : mLoop(nullptr),
      mIsExiting(false),
      mCallback(std::move(cb)),
      mThread(std::bind(&EventLoopThread::thread_function, this)),
      mMutex(),
      mCond()
{
}

EventLoopThread::~EventLoopThread()
{
    mIsExiting = true;
    if (mLoop)
    {
        mLoop->quit();
        mThread.join();
    }
}

EventLoop* EventLoopThread::startLoop()
{
    {
        std::unique_lock<std::mutex> lock{ mMutex };
        mCond.wait(lock, [this]() { return mLoop != nullptr; });
    }

    return mLoop;
}

void EventLoopThread::thread_function()
{
    EventLoop loop;

    if (mCallback)
    {
        mCallback(&loop);
    }

    {
        std::lock_guard<std::mutex> lock{ mMutex };
        mLoop = &loop;
        mCond.notify_one();
    }

    loop.loop();

    mLoop = nullptr;
}

} // namespace Net

} // namespace Asuka