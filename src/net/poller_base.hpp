// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_POLLER_BASE
#define ASUKA_POLLER_BASE

#include <map>
#include <memory>
#include <vector>

#include "../util/noncopyable.hpp"
#include "../util/time_stamp.hpp"

namespace Asuka
{

namespace Net
{

class Channel;
class EventLoop;

// PollerBase is base class Poller and Epoller
// it is for I/O mutiplexing
class PollerBase : Noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

public:
    PollerBase(EventLoop* loop);
    virtual ~PollerBase();

    // polls the I/O events
    // must be called in the mLoop's thread
    // `activeChannels` is a value-result
    virtual TimeStamp poll(int timeoutMs, ChannelList& activeChannels) = 0;

    // update the interested I/O events
    // must be called in the mLoop's thread
    virtual void update_channel(Channel& channel) = 0;

    // remove the channel when it is destroyed
    // must be called in the mLoop's thread
    virtual void remove_channel(Channel& channel) = 0;

    bool has_channel(const Channel& channel) const;

    static std::unique_ptr<PollerBase> create_default_poller(EventLoop* loop);

    void assert_in_loop_thread() const;

protected:
    // <fd, Channel*>
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap mChannels;

private:
    EventLoop* mLoop;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_POLLER_BASE
