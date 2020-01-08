#pragma once
#ifndef ASUKA_POLLER_HPP
#define ASUKA_POLLER_HPP

#include <poll.h>

#include <vector>

#include "poller_base.hpp"

namespace Asuka
{

namespace Net
{

// I/O multiplxing with poll(2)
class Poller : public PollerBase
{
public:
    Poller(EventLoop* loop);
    ~Poller() override;

    TimeStamp poll(int timeoutMs, ChannelList& activeChannels) override;

    void update_channel(Channel& channel) override;

    void remove_channel(Channel& channel) override;

private:
    void fill_active_channels(int numEvents, ChannelList& activeChannels) const;

private:
    std::vector<struct pollfd> mPollfdList;
};

} // namespace Asuka

} // namespace Asuka

#endif // ASUKA_POLLER_HPP
