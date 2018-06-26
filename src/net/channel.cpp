#include "channel.hpp"

#include <poll.h>

#include "../util/logger.hpp"
#include "event_loop.hpp"


namespace Asuka
{

namespace Net
{

const std::uint32_t Channel::kNoneEvent  = 0;
const std::uint32_t Channel::kReadEvent  = POLLIN | POLLPRI;
const std::uint32_t Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd) 
    : mLoop(loop), 
      mFd(fd),
      mEvents(0),
      mRevents(0),
      mIndex(-1),
      mIsTied(false),
      mIsEventHanding(false),
      mIsAddedInLoop(false),
      mIsLogHup(true)
{
}

Channel::~Channel()
{
    assert(!mIsAddedInLoop);
    assert(!mIsEventHanding);
    if (mLoop->is_in_loop_thread())
    {
        assert(!mLoop->has_channel(*this));
    }
}

void Channel::handle_event(TimeStamp receivedTime)
{
    std::shared_ptr<void> guard;
    if (mIsTied)
    {
        guard = mTie.lock();
        if (guard)
        {
            handle_event_with_guard(receivedTime);
        }
    }
    else
    {
        handle_event_with_guard(receivedTime);
    }
}

void Channel::set_read_callback(ReadEventCallback cb)
{
    mReadCallback = std::move(cb);
}

void Channel::set_write_callback(EventCallback cb)
{
    mWriteCallback = std::move(cb);
}

void Channel::set_close_callback(EventCallback cb)
{
    mCloseCallback = std::move(cb);
}

void Channel::set_error_callback(EventCallback cb)
{
    mErrorCallback = std::move(cb);
}


void Channel::tie(const std::shared_ptr<void>& sp)
{
    mTie = sp;
    mIsTied = true;
}


int Channel::get_fd() const noexcept
{
    return mFd;
}
std::uint32_t Channel::get_events() const noexcept
{
    return mEvents;
}
std::uint32_t Channel::get_revents() const noexcept
{
    return mRevents;
}
EventLoop* Channel::get_owner_loop() const
{
    return mLoop;
}

void Channel::set_revents(std::uint32_t revt)
{
    mRevents = revt;
}

bool Channel::is_none_event() const noexcept
{
    return mEvents == kNoneEvent;
}

void Channel::enable_read()
{
    mEvents |= kReadEvent;
    update();
}

void Channel::disable_read()
{
    mEvents &= ~kReadEvent;
    update();
}

void Channel::enable_write()
{
    mEvents |= kWriteEvent;
    update();
}

void Channel::disable_write()
{
    mEvents &= ~kWriteEvent;
    update();
}

void Channel::disable_all()
{
    mEvents = kNoneEvent;
    update();
}

bool Channel::is_reading() const
{
    return mEvents & kReadEvent;
}

bool Channel::is_writing() const
{
    return mEvents & kWriteEvent;
}

int Channel::get_index() const
{
    return mIndex;
}

void Channel::set_index(int idx)
{
    mIndex = idx;
}

std::string Channel::revents_to_string() const
{
    return static_events_to_string(mFd, mRevents);
}

std::string Channel::events_to_string() const
{
    return static_events_to_string(mFd, mEvents);
}

void Channel::set_not_loghup()
{
    mIsLogHup = false;
}

void Channel::remove()
{
    assert(is_none_event());
    mIsAddedInLoop = false;
    mLoop->remove_channel(*this);
}

std::string Channel::static_events_to_string(int fd, int events)
{
    std::string ret;
    ret.reserve(64);
    ret += std::to_string(fd);
    ret += ": ";
    if (events & POLLIN)
        ret += "IN ";
    if (events & POLLPRI)
        ret += "PRI ";
    if (events & POLLOUT)
        ret += "OUT ";
    if (events & POLLHUP)
        ret += "HUP ";
    if (events & POLLRDHUP)
        ret += "RDHUP ";
    if (events & POLLERR)
        ret += "ERR ";
    if (events & POLLNVAL)
        ret += "NVAL ";

    return ret;
}
void Channel::update()
{
    mIsAddedInLoop = true;
    mLoop->update_channel(*this);
}

void Channel::handle_event_with_guard(TimeStamp receivedTime)
{
    mIsEventHanding = true;
    LOG_TRACE << revents_to_string();

    if ((mRevents & POLLHUP) && !(mRevents & POLLIN))
    {
        // peer closed its end of the channel
        if (mIsLogHup)
        {
            LOG_WARN << "fd = " << mFd << " channel handle POLLHUP";
        }
        if (mCloseCallback)
        {
            mCloseCallback();
        }
    }

    if (mRevents & POLLNVAL)
    {
        // invalid request: fd not open
        LOG_WARN << "fd = " << mFd << " channel handle POLLNVAL";
    }

    if (mRevents & (POLLERR | POLLNVAL))
    {
        if (mErrorCallback)
        {
            mErrorCallback();
        }
    }

    if (mRevents & (POLLIN | POLLPRI | POLLRDHUP))
    {
        if (mReadCallback)
        {
            mReadCallback(receivedTime);
        }
    }
    
    if (mRevents & POLLOUT)
    {
        if (mWriteCallback)
        {
            mWriteCallback();
        }
    }

    mIsEventHanding = false;
}

} // namespace Net

} // namespace Asuka