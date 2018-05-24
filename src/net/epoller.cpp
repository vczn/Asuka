#include "epoller.hpp"

#include <poll.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>

#include "../util/logger.hpp"
#include "../util/time_stamp.hpp"
#include "channel.hpp"


namespace Asuka
{

namespace Net
{

static_assert(EPOLLHUP == POLLHUP, "EPOLLHUP != POLLHUP");
static_assert(EPOLLIN == POLLIN, "EPOLLIN != POLLIN");
static_assert(EPOLLPRI == POLLPRI, "EPOLLPRI != POLLPRI");
static_assert(EPOLLOUT == POLLOUT, "EPOLLOUT != POLLOUT");
static_assert(EPOLLRDHUP == POLLRDHUP, "EPOLLRDHUP != POLLHUP");
static_assert(EPOLLERR == POLLERR, "EPOLLERR != POLLERR");



Epoller::Epoller(EventLoop* loop)
    : PollerBase(loop),
      mEpollFd(::epoll_create1(EPOLL_CLOEXEC)),
      mEpollEvents(kInitEventListSize)
{
    if (mEpollFd < 0)
    {
        LOG_SYSFATAL << "epoll_create1 error";
    }
}

Epoller::~Epoller()
{
    close_fd(mEpollFd, "close epoll fd error");
}

TimeStamp Epoller::poll(int timeoutMs, ChannelList& activeChannels)
{
    LOG_TRACE << "total fd count = " << mChannels.size();
    int numEvents = ::epoll_wait(mEpollFd, mEpollEvents.data(),
        static_cast<int>(mEpollEvents.size()),
        timeoutMs);

    int saveErrno = errno;
    TimeStamp now = TimeStamp::now();

    if (numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happened";
        if (static_cast<std::size_t>(numEvents) == mEpollEvents.size())
        {
            mEpollEvents.resize(mEpollEvents.size() * 2);
        }
        fill_active_channels(numEvents, activeChannels);
    }
    else if (numEvents == 0)
    {
        LOG_TRACE << "epoll nothing happened";
    }
    else
    {
        if (saveErrno != EINTR)
        {
            errno = saveErrno;
            LOG_SYSERROR << "epoll wait error";
        }
    }

    return now;
}

void Epoller::update_channel(Channel& channel)
{
    PollerBase::assert_in_loop_thread();
    const std::size_t idx = channel.get_index();

    LOG_TRACE << "fd = " << channel.get_fd()
        << ", events = " << channel.get_events()
        << ", index = " << idx;

    if (idx == kNone || idx == kDeleted)
    {
        // a new, add the channel
        int fd = channel.get_fd();
        if (idx == kNone)
        {
            assert(mChannels.find(fd) != mChannels.end());
            mChannels[fd] = &channel;
        }
        else // idx == kDeleted
        {
            assert(mChannels.find(fd) == mChannels.end());
            assert(mChannels.at(fd) == &channel);
        }

        channel.set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else // idx == kAdded
    {
        // a existing one
        int fd = channel.get_fd();
        assert(mChannels.find(fd) != mChannels.end());
        assert(mChannels.at(fd) == &channel);
        (void)fd;

        if (channel.is_none_event())
        {
            // DEL
            update(EPOLL_CTL_DEL, channel);
        }
        else
        {
            // MOD
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Epoller::remove_channel(Channel& channel)
{
    PollerBase::assert_in_loop_thread();
    int fd = channel.get_fd();

    LOG_TRACE << "remove fd = " << fd;
    assert(mChannels.find(fd) != mChannels.end());
    assert(mChannels.at(fd) == &channel);
    assert(channel.is_none_event());

    std::size_t idx = channel.get_index();
    assert(idx = kAdded || idx == kDeleted);
    std::size_t n = mChannels.erase(fd);
    assert(n == 1);
    (void)n;

    if (idx == kAdded)
    {
        update(EPOLL_CTL_DEL, channel);
    }

    channel.set_index(kNone);
}

void Epoller::fill_active_channels(int numEvents, 
    ChannelList& activeChannels) const
{
    assert(static_cast<std::size_t>(numEvents) <= mEpollEvents.size());

    for (int i = 0; i < numEvents; ++i)
    {
        Channel* channel = static_cast<Channel*>(mEpollEvents[i].data.ptr);
#ifndef NDEBUG
        int fd = channel->get_fd();
        auto iter = mChannels.find(fd);
        assert(iter != mChannels.end());
        assert(iter->second == channel);
#endif // NDEBUG
        channel->set_revents(mEpollEvents[i].events);
        activeChannels.push_back(channel);
    }
}

void Epoller::update(int op, Channel& channel)
{
    struct epoll_event evt;
    ::bzero(&evt, sizeof(evt));

    evt.events = channel.get_events();
    evt.data.ptr = &channel;

    int fd = channel.get_fd();

    LOG_TRACE << "epoll_ctl_op = " << operation_to_string(op)
        << ", fd = " << fd << ", events = {"
        << channel.events_to_string() << " }";

    if (::epoll_ctl(mEpollFd, op, fd, &evt) < 0)
    {
        // error
        if (op == EPOLL_CTL_DEL)
        {
            LOG_SYSERROR << "epoll_ctl DEL fd = " << fd << " error";
        }
        else
        {
            LOG_SYSFATAL << "epoll_ctl" << operation_to_string(op)
                << " fd = " << fd << " error";
        }
    }
}

const char* Epoller::operation_to_string(int op)
{
    switch (op)
    {
    case EPOLL_CTL_ADD:
        return "ADD";
    case EPOLL_CTL_MOD:
        return "MOD";
    case EPOLL_CTL_DEL:
        return "DEL";
    }

    return "UNKNOWN OPERATION";
}

} // namespace Net

} // namespace Asuka