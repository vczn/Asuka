#include "tcp_connection.hpp"

#include <netinet/tcp.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <functional>

#include "../util/logger.hpp"
#include "../util/string_view.hpp"
#include "../util/weak_callback.hpp"
#include "buffer.hpp"
#include "channel.hpp"
#include "event_loop.hpp"
#include "socket.hpp"

namespace Asuka
{
    
namespace Net
{

void default_connection_callback(const TcpConnectionPtr& conn)
{
    LOG_TRACE << conn->get_local_address().get_ipport() << " -> "
        << conn->get_peer_address().get_ipport() << " is "
        << (conn->connected() ? "UP" : "DOWN");
    // be able to register message callback only
}

void default_message_callback(const TcpConnectionPtr& conn, Buffer& buf, TimeStamp ts)
{
    (void)conn;
    (void)ts;
    buf.retrieve_all();     // discard
}

TcpConnection::TcpConnection(EventLoop* loop, 
                             std::string name, 
                             int sockfd,
                             const IpPort& localAddr, 
                             const IpPort& peerAddr)
    : mLoop(loop), 
      mName(std::move(name)),
      mStatus(kIsConnecting),
      mIsReading(true),
      mSocket(new Socket{sockfd}),
      mChannel(new Channel{mLoop, sockfd}),
      mLocalAddr(localAddr),
      mPeerAddr(peerAddr), 
      mHighWaterMark(60 * 1024 * 1024)
{
    mChannel->set_read_callback(std::bind(&TcpConnection::handle_read, 
        this, std::placeholders::_1));
    mChannel->set_write_callback(std::bind(&TcpConnection::handle_write,
        this));
    mChannel->set_close_callback(std::bind(&TcpConnection::handle_close,
        this));
    mChannel->set_error_callback(std::bind(&TcpConnection::handle_error,
        this));
    LOG_DEBUG << "TcpConnection ctor[" << mName << "] at " << this
        << " fd = " << sockfd;
    mSocket->set_keep_alive(1);
}

TcpConnection::~TcpConnection()
{
    LOG_DEBUG << "TcpConnection dtor[" << mName << "] at " << this
        << " fd = " << mChannel->get_fd()
        << " status = " << status_to_string();
    assert(mStatus == kDisConnected);
}

EventLoop* TcpConnection::get_loop()
{
    return mLoop;
}

const std::string& TcpConnection::get_name() const
{
    return mName;
}

const IpPort& TcpConnection::get_local_address() const
{
    return mLocalAddr;
}

const IpPort& TcpConnection::get_peer_address() const
{
    return mPeerAddr;
}

bool TcpConnection::connected() const
{
    return mStatus == kConnected;
}

bool TcpConnection::disconnected() const
{
    return mStatus == kDisConnected;
}

bool TcpConnection::get_tcp_info(struct tcp_info* tcpInfo) const
{
    // TODO
    return true;
}

std::string TcpConnection::get_tcp_info_to_string() const
{
    // TODO
    return std::string();
}

void TcpConnection::send(const void* message, std::size_t len)
{
    send(StringView{ static_cast<const char*>(message), len });
}

void TcpConnection::send(const StringView& message)
{
    if (mStatus == kConnected)
    {
        if (mLoop->is_in_loop_thread())
        {
            send_in_loop(message.data(), message.size());
        }
        else
        {
            mLoop->run_in_loop(std::bind(
                [](std::string str, TcpConnectionPtr ptr)
                { ptr->send_in_loop(str.data(), str.size()); }, 
                    message.to_string(), shared_from_this()));          // MOD
        }
    }
}

void TcpConnection::send(Buffer& message)
{
    if (mStatus == kConnected)
    {
        if (mLoop->is_in_loop_thread())
        {
            send_in_loop(message.read_begin(), message.readable_bytes());
            message.retrieve_all();
        }
        else
        {
            mLoop->run_in_loop(std::bind(
                [](std::string str, TcpConnectionPtr ptr)
                { ptr->send_in_loop(str.data(), str.size()); },
                    message.retrieve_all_as_string(), shared_from_this())); // MOD
        }
    }
}


void TcpConnection::shutdown()
{
    // FIXME: use compare and swap
    if (mStatus == kConnected)
    {
        set_status(kIsDisConnecting);
        mLoop->run_in_loop(
            std::bind(&TcpConnection::shutdown_in_loop, this));
    }
}

void TcpConnection::force_close()
{
    // FIXME: use compare and swap
    if (mStatus == kConnected || mStatus == kIsDisConnecting)
    {
        set_status(kIsDisConnecting);
        mLoop->queue_in_loop(
            std::bind(&TcpConnection::force_close_in_loop, 
            shared_from_this()));
    }
}

void TcpConnection::force_close_with_delay(double seconds)
{
    if (mStatus == kConnected || mStatus == kIsDisConnecting)
    {
        set_status(kIsDisConnecting);
        mLoop->run_after(
            seconds,
            make_weak_callback(shared_from_this(),
                               &TcpConnection::force_close));
    }
}

void TcpConnection::set_tcp_no_delay()
{
    mSocket->set_no_delay(1);
}

void TcpConnection::start_read()
{
    mLoop->run_in_loop(std::bind(&TcpConnection::start_read_in_loop, this));
}

void TcpConnection::stop_read()
{
    mLoop->run_in_loop(std::bind(&TcpConnection::stop_read_in_loop, this));
}

bool TcpConnection::is_reading() const
{
    return mIsReading;
}

void TcpConnection::set_context(const Any& context)
{
    mContext = context;
}

const Any& TcpConnection::get_context() const
{
    return mContext;
}

Any& TcpConnection::get_context()
{
    return mContext;
}

void TcpConnection::set_connection_callback(ConnectionCallback cb)
{
    mConnectionCallback = std::move(cb);
}

void TcpConnection::set_message_callback(MessageCallback cb)
{
    mMessageCallback = std::move(cb);
}

void TcpConnection::set_write_complete_callback(WriteCompleteCallback cb)
{
    mWriteCompleteCallback = std::move(cb);
}

void TcpConnection::set_high_water_mark_callback(HighWaterMarkCallback cb)
{
    mHighWaterMarkCallback = std::move(cb);
}

void TcpConnection::set_close_callback(CloseCallback cb)
{
    mCloseCallback = std::move(cb);
}

Buffer& TcpConnection::get_input_buffer()
{
    return mInputBuffer;
}

Buffer& TcpConnection::get_output_buffer()
{
    return mOutputBuffer;
}

void TcpConnection::connect_established()
{
    mLoop->assert_in_loop_thread();
    assert(mStatus == kIsConnecting);
    set_status(kConnected);
    mChannel->tie(shared_from_this());
    mChannel->enable_read();

    mConnectionCallback(shared_from_this());
}

void TcpConnection::connect_destroy()
{
    mLoop->assert_in_loop_thread();
    if (mStatus == kConnected)
    {
        set_status(kDisConnected);
        mChannel->disable_all();

        mConnectionCallback(shared_from_this());
    }
    mChannel->remove();
}

void TcpConnection::handle_read(TimeStamp receivedTime)
{
    mLoop->assert_in_loop_thread();
    int saveErrno = 0;
    ssize_t n = mInputBuffer.read_fd(mChannel->get_fd(), saveErrno);
    if (n > 0)
    {
        mMessageCallback(shared_from_this(), mInputBuffer, receivedTime);
    }
    else if (n == 0)
    {
        handle_close();
    }
    else
    {
        errno = saveErrno;
        LOG_SYSERROR << "TcpConnection::handle_read";
        handle_error();
    }
}

void TcpConnection::handle_write()
{
    mLoop->assert_in_loop_thread();
    if (mChannel->is_writing())
    {
        std::size_t n = ::write(mChannel->get_fd(),
            mOutputBuffer.read_begin(), mOutputBuffer.readable_bytes());
        if (n > 0)
        {
            mOutputBuffer.retrieve(n);
            if (mOutputBuffer.readable_bytes() == 0)    // write completely
            {
                mChannel->disable_write();
                if (mWriteCompleteCallback)
                {
                    mLoop->queue_in_loop(std::bind(
                        mWriteCompleteCallback, shared_from_this()));
                }

                if (mStatus == kIsDisConnecting)
                {
                    shutdown_in_loop();
                }   
            }
        }
        else    // n <= 0
        {
            LOG_SYSERROR << "TcpConnection::handle_write";
        }
    }
    else  // !is_writing
    {
        LOG_TRACE << "connection fd = " << mChannel->get_fd()
            << "is down, no more writing";
    }
}

void TcpConnection::handle_close()
{
    mLoop->assert_in_loop_thread();
    LOG_TRACE << "fd = " << mChannel->get_fd() 
        << " status = " << status_to_string();
    assert(mStatus == kConnected || mStatus == kIsDisConnecting);
    set_status(kDisConnected);
    mChannel->disable_all();
    
    TcpConnectionPtr guardThis(shared_from_this());
    mConnectionCallback(guardThis);

    mCloseCallback(guardThis);
}

void TcpConnection::handle_error()
{
    mLoop->assert_in_loop_thread();
    int err = get_socket_error(mChannel->get_fd());
    LOG_ERROR << "TcpConnection::handle_error [" << mName
        << "] - SO_ERROR: " << errno_to_string_r(err);
}

void TcpConnection::send_in_loop(const void* data, std::size_t len)
{
    mLoop->assert_in_loop_thread();
    if (mStatus == kDisConnected)
    {
        LOG_WARN << "disconnected, give up writing";
        return;
    }

    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    // if output queue is empty, try writing directly
    if (!mChannel->is_writing() && mOutputBuffer.readable_bytes() == 0)
    {
        nwrote = ::write(mChannel->get_fd(), data, len);
        if (nwrote < 0)     // error
        {
            nwrote = 0;
            if (errno != EWOULDBLOCK)
            {
                LOG_SYSERROR << "TcpConnection::send_in_loop";
                if (errno == ECONNRESET || errno == EPIPE) // FIXME: any other?
                {
                    faultError = true;
                }
            }
        }
        else  // nwrote >= 0
        {
            remaining = len - static_cast<std::size_t>(nwrote);
            if (remaining == 0 && mWriteCompleteCallback)
            {
                mLoop->queue_in_loop(std::bind(mWriteCompleteCallback,
                    shared_from_this()));
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0)
    {
        std::size_t oldLen = mOutputBuffer.readable_bytes();
        if (oldLen + remaining >= mHighWaterMark
            && oldLen < mHighWaterMark
            && mHighWaterMarkCallback)
        {
            mLoop->queue_in_loop(std::bind(mHighWaterMarkCallback, 
                shared_from_this(), oldLen + remaining));
        }
        mOutputBuffer.append(static_cast<const char*>(data) + nwrote, remaining);
        if (!mChannel->is_writing())
        {
            mChannel->enable_write();
        }
    }
}


void TcpConnection::shutdown_in_loop()
{
    mLoop->assert_in_loop_thread();
    if (!mChannel->is_writing())
    {
        mSocket->shutdown_write();
    }
}

void TcpConnection::force_close_in_loop()
{
    mLoop->assert_in_loop_thread();
    if (mStatus == kConnected || mStatus == kIsDisConnecting)
    {
        handle_close();
    }
}

void TcpConnection::set_status(Status s)
{
    mStatus = s;
}

const char* TcpConnection::status_to_string() const
{
    switch (mStatus)
    {
    case kDisConnected:
        return "disconnected";
    case kIsConnecting:
        return "is conntecting";
    case kConnected:
        return "connected";
    case kIsDisConnecting:
        return "is disconnected";
    }

    return "unknown status";
}

void TcpConnection::start_read_in_loop()
{
    mLoop->assert_in_loop_thread();
    if (!mIsReading || !mChannel->is_reading())
    {
        mChannel->enable_read();
        mIsReading = true;
    }
}

void TcpConnection::stop_read_in_loop()
{
    mLoop->assert_in_loop_thread();
    if (mIsReading || mChannel->is_reading())
    {
        mChannel->disable_read();
        mIsReading = false;
    }
}

} // namespace Net

} // namespace Asuka