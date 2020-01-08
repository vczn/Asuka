#pragma once
#ifndef ASUKA_EVENT_LOOP_THREAD_HPP
#define ASUKA_EVENT_LOOP_THREAD_HPP

#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

#include "../util/noncopyable.hpp"

namespace Asuka
{

namespace Net
{

class EventLoop;

class EventLoopThread : Noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

public:
    EventLoopThread(ThreadInitCallback cb = ThreadInitCallback());

    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void thread_function();

private:
    EventLoop* mLoop;
    bool mIsExiting;
    ThreadInitCallback mCallback;
    std::thread mThread;
    std::mutex mMutex;
    std::condition_variable mCond;
};

} // namespace Net

} // namespace Asuka



#endif // ASUKA_EVENT_LOOP_THREAD_H