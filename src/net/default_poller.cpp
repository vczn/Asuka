#include "poller_base.hpp"

#include "../util/config.hpp"
#include "epoller.hpp"
#include "poller.hpp"

namespace Asuka
{

namespace Net
{

std::unique_ptr<PollerBase> PollerBase::create_default_poller(EventLoop* loop)
{
    std::unique_ptr<PollerBase> result{ nullptr };
    bool useEpoll = Config::instance().get_use_epoll();
    if (useEpoll)
    {
        result.reset(new Epoller{ loop });
    }
    else
    {
        result.reset(new Poller{ loop });
    }

    return result;
}

} // namespace Net

} // namespace Asuka