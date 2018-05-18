// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_CHANNEL_HPP
#define ASUKA_CHANNEL_HPP

#include <functional>
#include <memory>
#include <string>

#include "../util/noncopyable.hpp"
#include "../util/time_stamp.hpp"

namespace Asuka
{

namespace Net
{

class EventLoop;


// a selectable I/O channel
// the Channel object doesn't own the file descriptor
// The file descriptor coule be a sockfd, 
// an eventfd, a timerfd, or a singlefd
class Channel : Noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(TimeStamp)>;
public:

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handle_read(TimeStamp receivedTime);
    
    void set_read_callback(ReadEventCallback cb);
    void set_write_callback(EventCallback cb);
    void set_close_callback(EventCallback cb);
    void set_error_callback(EventCallback cb);

    // tie the channel to the owner object managed by shared_ptr 
    // prevent the owner object is destroyed in handle event
    void tie(const std::shared_ptr<void>& sp);

    int get_fd() const noexcept;
    short get_events() const noexcept;
    short get_revents() const noexcept;
    EventLoop* get_owner_loop() const;

    void set_revents(short revt);   // used by poller

    bool is_none_event() const noexcept;

    void enable_read();
    void disable_read();
    void enable_write();
    void disable_write();
    void disable_all();

    bool is_reading() const;
    bool is_writing() const;

    // for poller
    std::size_t get_index() const;
    void set_index(std::size_t idx);

    // for debuging
    std::string revents_to_string() const;
    std::string events_to_string() const;

    void set_cannot_log();

    void remove();

private:
    static std::string static_events_to_string(int fd, int events);

    void update();
    void handle_event_with_guard(TimeStamp receivedTime);

    static const short kNoneEvent;
    static const short kReadEvent;
    static const short kWriteEvent;

private:
    EventLoop* mLoop;
    const int mFd;

    short mEvents;
    short mRevents;
    std::size_t mIndex;     // for poller

    std::weak_ptr<void> mTie;
    bool mIsTied;
    bool mIsEventHanding;
    bool mIsAddedInLoop;
    bool mCanLog;

    ReadEventCallback mReadCallback;
    EventCallback mWriteCallback;
    EventCallback mCloseCallback;
    EventCallback mErrorCallback;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_CHANNEL_HPP