// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

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
    EventLoopThread(ThreadInitCallback cb = ThreadInitCallback(),
        const std::string& name = std::string());

    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void thread_function();

private:
    EventLoop* mLoop;
    bool mIsExiting;
    std::thread mThread;
    std::mutex mMutex;
    std::condition_variable mCond;
    ThreadInitCallback mCallback;
};

} // namespace Net

} // namespace Asuka



#endif // ASUKA_EVENT_LOOP_THREAD_H