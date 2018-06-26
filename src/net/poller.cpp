#include "poller.hpp"

#include <cassert>

#include "../util/logger.hpp"
#include "channel.hpp"

namespace Asuka
{

namespace Net
{

Poller::Poller(EventLoop* loop)
    : PollerBase(loop)
{
}

Poller::~Poller()
{
}

TimeStamp Poller::poll(int timeoutMs, ChannelList& activeChannels)
{
    int numEvents = ::poll(mPollfdList.data(), mPollfdList.size(), timeoutMs);
    TimeStamp now = TimeStamp::now();

    if (numEvents > 0)
    {
        LOG_TRACE << numEvents << " events happened in poll()";
        fill_active_channels(numEvents, activeChannels);
    }
    else if (numEvents == 0)
    {
        LOG_TRACE << "nothing happened in poll()";
    }
    else // numEvents < 0
    {
        if (errno != EINTR)
        {
            LOG_SYSERROR << "poll error";
        }
    }

    return now;
}

void Poller::update_channel(Channel& channel)
{
    PollerBase::assert_in_loop_thread();

    LOG_TRACE << "fd = " << channel.get_fd()
        << "events = " << channel.get_events();

    if (channel.get_index() < 0)
    {
        // a new one, add the pollfd
        assert(mChannels.find(channel.get_fd()) == mChannels.end());
        struct pollfd pfd;
        pfd.fd = channel.get_fd();
        pfd.events = static_cast<short>(channel.get_events());
        pfd.revents = 0;

        mPollfdList.push_back(pfd);
        assert(!mPollfdList.empty());
        int idx = static_cast<int>(mPollfdList.size() - 1);
        channel.set_index(idx);
        mChannels[pfd.fd] = &channel;
    }
    else
    {
        // update existing one
        assert(mChannels.find(channel.get_fd()) != mChannels.end());
        assert(mChannels.at(channel.get_fd()) == &channel);
        std::size_t idx = channel.get_index();
        assert(idx < mPollfdList.size());

        struct pollfd& pfd = mPollfdList[idx];
        assert(pfd.fd == channel.get_fd() || pfd.fd == -channel.get_fd() - 1);
        pfd.fd = channel.get_fd();
        pfd.events = static_cast<short>(channel.get_events());
        pfd.revents = 0;

        if (channel.is_none_event())
        {
            // ignore the pollfd
            pfd.fd = -channel.get_fd() - 1;
        }
    }
}

void Poller::remove_channel(Channel& channel)
{
    PollerBase::assert_in_loop_thread();

    LOG_TRACE << "fd = " << channel.get_fd();
    assert(mChannels.find(channel.get_fd()) != mChannels.end());
    assert(mChannels.at(channel.get_fd()) == &channel);
    assert(channel.is_none_event());

    int idx = channel.get_index();
    assert(idx < static_cast<int>(mPollfdList.size()));
    const struct pollfd& pfd = mPollfdList[idx];
    assert(pfd.fd == -channel.get_fd() - 1);
    assert(pfd.events == static_cast<short>(channel.get_events()));
    (void)pfd;

    std::size_t n = mChannels.erase(channel.get_fd());
    assert(n == 1);
    (void)n;

    // remove pollfd from `mPollfdList`
    if (idx == static_cast<int>(mPollfdList.size() - 1))
    {
        // the pollfd which will be removed at the end
        mPollfdList.pop_back();
    }
    else
    {
        int fdAtEnd = mPollfdList.back().fd;
        // swap the pollfd which will be removed and the pollfd at the end
        std::iter_swap(mPollfdList.begin() + idx, mPollfdList.end() - 1);
        if (fdAtEnd < 0)
        {
            fdAtEnd = -fdAtEnd - 1;
        }

        // swap idx
        mChannels[fdAtEnd]->set_index(idx);
        mPollfdList.pop_back();
    }
}

void Poller::fill_active_channels(int numEvents, 
    ChannelList& activeChannels) const
{
    for (auto pfd = mPollfdList.begin(); pfd != mPollfdList.end(); ++pfd)
    {
        if (pfd->revents > 0)
        {
            --numEvents;
            auto iter = mChannels.find(pfd->fd);
            assert(iter != mChannels.end());
            Channel* channel = iter->second;
            assert(channel->get_fd() == pfd->fd);
            channel->set_revents(pfd->revents);
            activeChannels.push_back(channel);
        }
    }
}

} // namespace Net

} // namespace Asuka