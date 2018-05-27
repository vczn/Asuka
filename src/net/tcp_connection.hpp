// Part of Asuka net utility, public header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_TCP_CONNECTION_HPP
#define ASUKA_TCP_CONNECTION_HPP

#include <atomic>
#include <memory>

#include <netinet/tcp.h>

#include "../util/any.hpp"
#include "../util/noncopyable.hpp"
#include "buffer.hpp"
#include "callback.hpp"
#include "ip_port.hpp"

namespace Asuka
{

namespace Net
{

class Channel;
class EventLoop;
class Socket;

class TcpConnection : Noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
public:
    TcpConnection(EventLoop* loop, 
                  std::string name, 
                  int sockfd, 
                  const IpPort& localAddr, 
                  const IpPort& peerAddr);
    ~TcpConnection();

    const EventLoop* get_loop() const;
    const std::string& get_name() const;
    const IpPort& get_local_address() const;
    const IpPort& get_peer_address() const;

    bool connected() const;
    bool disconnected() const;

    // return true if success
    bool get_tcp_info(struct tcp_info* tcpInfo) const;
    std::string get_tcp_info_to_string() const;

    void send(const void* message, int len);
    void send(const StringView& message);
    void send(std::string&& message);
    void send(const Buffer& message);  // will swap data
    void send(Buffer&& message);

    // FIXME not thread safe, no simultaneous calling
    void shutdown();
    void force_close();
    void force_close_with_delay(double seconds);

    void set_tcp_no_delay();

    void start_read();
    void stop_read();
    bool is_reading() const;

    void set_context();
    const Any& get_context() const;
    Any& get_context();

    void set_connection_callback(ConnectionCallback cb);
    void set_message_callback(MessageCallback cb);
    void set_write_complete_callback(WriteCompleteCallback cb);
    void set_high_water_mark_callback(HighWaterMarkCallback cb);
    void set_close_callback(CloseCallback cb);

    Buffer& get_input_buffer();
    Buffer& get_output_buffer();

    // calls when TcpServer accept a new connection
    // should be called only once
    void connect_established();

    // calls when TcpServer remove a connection from its map
    // should be called only once
    void connect_destroy();

public:
    using Status = std::uint8_t;
private:
    static const Status kDisConnected    = 0;
    static const Status kIsConnecting    = 1;
    static const Status kConnected       = 2;
    static const Status kIsDisConnecting = 3;
private:

    EventLoop* mLoop;
    const std::string mName;
    std::atomic<Status> mStatus;
    bool mIsReading;

    std::unique_ptr<Socket> mSocket;
    std::unique_ptr<Channel> mChannel;
    const IpPort mLocalAddr;
    const IpPort mPeerAddr;

    ConnectionCallback mConnectionCallback;
    MessageCallback mMessageCallback;
    WriteCompleteCallback mWriteCompleteCallback;
    HighWaterMarkCallback mHighWaterMarkCallback;
    CloseCallback mCloseCallback;

    std::size_t mHighWaterMark;
    Buffer mInputBuffer;
    Buffer mOutputBuffer;       // FIXME use list<Buffer>
    Any mContext;
};

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

} // namespace Net

} // namespace Asuka

#endif // ASUKA_TCP_CONNECTION_HPP