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

    void handle_event(TimeStamp receivedTime);
    
    void set_read_callback(ReadEventCallback cb);
    void set_write_callback(EventCallback cb);
    void set_close_callback(EventCallback cb);
    void set_error_callback(EventCallback cb);

    // tie the channel to the owner object managed by shared_ptr 
    // prevent the owner object is destroyed in handle event
    void tie(const std::shared_ptr<void>& sp);

    int get_fd() const noexcept;
    std::uint32_t get_events() const noexcept;
    std::uint32_t get_revents() const noexcept;
    EventLoop* get_owner_loop() const;

    void set_revents(std::uint32_t revt);   // used by poller

    bool is_none_event() const noexcept;

    void enable_read();
    void disable_read();
    void enable_write();
    void disable_write();
    void disable_all();

    bool is_reading() const;
    bool is_writing() const;

    // for poller
    int get_index() const;
    void set_index(int idx);

    // for debuging
    std::string revents_to_string() const;
    std::string events_to_string() const;

    void set_not_loghup();

    void remove();

private:
    static std::string static_events_to_string(int fd, int events);

    void update();
    void handle_event_with_guard(TimeStamp receivedTime);

    static const std::uint32_t kNoneEvent;
    static const std::uint32_t kReadEvent;
    static const std::uint32_t kWriteEvent;

private:
    EventLoop* mLoop;
    const int mFd;

    std::uint32_t mEvents;
    std::uint32_t mRevents;
    int mIndex;     // for poller

    std::weak_ptr<void> mTie;
    bool mIsTied;
    bool mIsEventHanding;
    bool mIsAddedInLoop;
    bool mIsLogHup;

    ReadEventCallback mReadCallback;
    EventCallback mWriteCallback;
    EventCallback mCloseCallback;
    EventCallback mErrorCallback;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_CHANNEL_HPP