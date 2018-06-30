#include "connector.hpp"

#include <cassert>
#include <cerrno>

#include "../util/logger.hpp"
#include "channel.hpp"
#include "event_loop.hpp"
#include "socket.hpp"

namespace Asuka
{

namespace Net
{

const std::uint32_t Connector::kMaxRetryMs = 30000; // 30s
const std::uint32_t Connector::kInitRetryMs = 500;  // 0.5s

Connector::Connector(EventLoop* loop, const IpPort& servaddr)
    : mLoop(loop),
      mServAddr(servaddr),
      mStatus(kDisConnected),
      mIsConnect(false),
      mRetryMs(kInitRetryMs),
      mChannel(nullptr)
{
    LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector()
{
    LOG_DEBUG << "dtor[" << this << "]";
    assert(!mChannel);
}

void Connector::set_connection_callback(ConnectionCallback cb)
{
    mConnectionCallback = std::move(cb);
}

void Connector::start()
{
    mIsConnect = true;
    // ??? FIXME unsafe
    mLoop->run_in_loop(std::bind(&Connector::start_in_loop, this));
}

void Connector::stop()
{
    mIsConnect = false;
    // ??? FIXME unsafe
    mLoop->queue_in_loop(std::bind(&Connector::stop_in_loop, this));
    // ??? FIXME cancel timer
}

void Connector::restart()
{
    mLoop->assert_in_loop_thread();
    set_status(kDisConnected);
    mRetryMs = kInitRetryMs;
    mIsConnect = true;
    start_in_loop();
}

const IpPort& Connector::get_server_address() const
{
    return mServAddr;
}

void Connector::set_status(Status s)
{
    mStatus = s;
}

void Connector::start_in_loop()
{
    mLoop->assert_in_loop_thread();
    assert(mStatus == kDisConnected);
    if (mIsConnect)
    {
        connect();
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

void Connector::stop_in_loop()
{
    mLoop->assert_in_loop_thread();
    if (mStatus == kIsConnecting)
    {
        set_status(kDisConnected);
        // !!!
        LOG_DEBUG << "connector remove_and_reset_channel";
        int sockfd = remove_and_reset_channel();
        retry(sockfd);
    }
}

void Connector::connect()
{
    int sockfd = create_nonblock_socket(mServAddr.get_family());
    socklen_t len = mServAddr.get_address_length();
    int ret = ::connect(sockfd, mServAddr.get_sockaddr(), len);
    int saveErrno = (ret == 0) ? 0 : errno;
    switch (saveErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        connecting(sockfd);
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        retry(sockfd);
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_SYSERROR << "connect error";
        close_sockfd(sockfd);
        break;

    default:
        LOG_SYSERROR << "unexpected error in connect";
        close_sockfd(sockfd);
        break;
    }
}

void Connector::connecting(int sockfd)
{
    set_status(kIsConnecting);
    assert(!mChannel);
    mChannel.reset(new Channel(mLoop, sockfd));
    // FIXME unsafe
    mChannel->set_write_callback(
        std::bind(&Connector::handle_write, this));
    // FIXME unsafe
    mChannel->set_error_callback(
        std::bind(&Connector::handle_error, this));

    mChannel->enable_write();
}

void Connector::handle_write()
{
    LOG_TRACE << "Connector::handle_write status = " 
        << status_to_string();
    if (mStatus == kIsConnecting)
    {
        int sockfd = remove_and_reset_channel();
        int err = get_socket_error(sockfd);
        if (err)
        {
            LOG_WARN << "SO_ERROR = " << err << ": " 
                << errno_to_string_r(err);
            retry(sockfd);
        }
        else
        {
            set_status(kConnected);
            if (mIsConnect)
            {
                mConnectionCallback(sockfd);
            }
            else
            {
                LOG_DEBUG << "is_connect is not true";
                close_sockfd(sockfd);
            }
        }
    }
    else
    {
        LOG_DEBUG << "status is not kIsConnecting";
    }
}

void Connector::handle_error()
{
    LOG_ERROR << "Connector::handle_error status = " << status_to_string();
    if (mStatus == kIsConnecting)
    {
        int sockfd = remove_and_reset_channel();
        int err = get_socket_error(sockfd);

        LOG_TRACE << "SO_ERROR" << err << "  " << errno_to_string_r(err);
        retry(sockfd);
    }
}

void Connector::retry(int sockfd)
{
    close_sockfd(sockfd);
    set_status(kDisConnected);
    if (mIsConnect)
    {
        LOG_INFO << "Retry connect to " << mServAddr.get_ipport()
            << " in " << mRetryMs << " milliseconds";
        mLoop->run_after(mRetryMs / 1000.0,
            std::bind(&Connector::start_in_loop, shared_from_this()));
        mRetryMs = std::min(mRetryMs * 2, kMaxRetryMs);
    }
    else
    {
        LOG_DEBUG << "do not connect";
    }
}

int Connector::remove_and_reset_channel()
{
    LOG_DEBUG << "disable_all and remove";
    mChannel->disable_all();
    mChannel->remove();
    int sockfd = mChannel->get_fd();

    // ??? FIXME unsafe
    mLoop->queue_in_loop(
        std::bind(&Connector::reset_channel, this));

    return sockfd;
}

void Connector::reset_channel()
{
    mChannel.reset();
}

const char* Connector::status_to_string() const
{
    switch (mStatus.load())
    {
    case kDisConnected:
        return "disconnected";
    case kIsConnecting:
        return "is connecting";
    case kConnected:
        return "conected";
    }
    return "unknown status";
}


} // namespace Net

} // namespace Asuka