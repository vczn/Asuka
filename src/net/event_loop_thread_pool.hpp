// Part of Asuka net utility, public header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_EVENT_LOOP_THREAD_POOL_HPP
#define ASUKA_EVENT_LOOP_THREAD_POOL_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "../util/noncopyable.hpp"

namespace Asuka
{

namespace Net
{

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool : Noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThreadPool(EventLoop* baseLoop, std::string name);
    ~EventLoopThreadPool();

    // call it before calling `start()`
    void set_thread_number(std::size_t num);
    void start(ThreadInitCallback cb = ThreadInitCallback());

    // vaild after calling `start()`
    // round-robin
    EventLoop* get_next_loop();

    EventLoop* get_loop_for_hash(std::size_t hashCode);

    std::vector<EventLoop*> get_all_loops();

    bool started() const;
    const std::string& get_name() const;

private:
    EventLoop* mBaseLoop;
    std::string mName;
    bool mIsStarted;
    std::size_t mNumThreads;
    int mNext;
    std::vector<std::unique_ptr<EventLoopThread>> mThreads;
    std::vector<EventLoop*> mLoops;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_EVENT_LOOP_THREAD_POOL_HPP