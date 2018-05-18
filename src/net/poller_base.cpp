#include "poller_base.hpp"

#include "channel.hpp"
#include "event_loop.hpp"

namespace Asuka
{

namespace Net
{

PollerBase::PollerBase(EventLoop* loop)
    : mLoop(loop)
{
}

PollerBase::~PollerBase()
{
}

bool PollerBase::has_channel(const Channel& channel) const
{
    assert_in_loop_thread();
    auto iter = mChannels.find(channel.get_fd());

    return iter != mChannels.end() && iter->second == &channel;
}

void PollerBase::assert_in_loop_thread() const
{
    return mLoop->assert_in_loop_thread();
}

} // namespace Net

} // namespace Asuka