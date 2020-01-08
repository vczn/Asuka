#pragma once
#ifndef ASUKA_EPOLLER_HPP
#define ASUKA_EPOLLER_HPP

#include <sys/epoll.h>
#include <vector>

#include "poller_base.hpp"

namespace Asuka
{

namespace Net
{

class Epoller : public PollerBase
{
public:
    Epoller(EventLoop* loop);
    ~Epoller() override;

    // epoll_wait
    TimeStamp poll(int timeoutMs, ChannelList& activeChannels) override;

    void update_channel(Channel& channel) override;

    void remove_channel(Channel& channel) override;

private:
    static const std::size_t kInitEventListSize = 32;
    static const int kNew     = -1;
    static const int kAdded   = 1;
    static const int kDeleted = 2;

private:
    void fill_active_channels(int numEvents, ChannelList& activeChannels) const;
    void update(int op, Channel& channel);
    const char* operation_to_string(int op);

private:
    int mEpollFd;
    std::vector<epoll_event> mEpollEvents;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_EPOLLER_HPP
