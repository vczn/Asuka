// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_CONNECTOR_HPP
#define ASUKA_CONNECTOR_HPP


#include <atomic>
#include <functional>
#include <memory>

#include "../util/noncopyable.hpp"
#include "ip_port.hpp"


namespace Asuka
{

namespace Net
{

class Channel;
class EventLoop;

// `Connector` is used by `TcpClient` to 
// connect the `TcpServer`
class Connector : Noncopyable, 
                  public std::enable_shared_from_this<Connector>
{
public:
    using ConnectionCallback = std::function<void(int sockfd)>;

    using Status = std::uint8_t;

    static const Status kDisConnected = 0;
    static const Status kIsConnecting = 1;
    static const Status kConnected = 2;

    static const std::uint32_t kMaxRetryMs;   // 30s
    static const std::uint32_t kInitRetryMs;  // 0.5s

public:
    Connector(EventLoop* loop, const IpPort& servaddr);
    ~Connector();

    void set_connection_callback(ConnectionCallback cb);

    void start();   // can be called in any thread
    void stop();    // can be called in any thread
    void restart(); // must be called in the loop thread

    const IpPort& get_server_address() const;

private:
    void set_status(Status s);
    void start_in_loop();
    void stop_in_loop();

    // call `::connect`
    void connect();

    // set `mStatus` and `mChannel`
    void connecting(int sockfd);

    void handle_write();
    void handle_error();
    void retry(int sockfd);
    int remove_and_reset_channel();
    void reset_channel();

    const char* status_to_string() const;
private:
    EventLoop* mLoop;
    
    IpPort mServAddr;
    std::atomic<Status> mStatus;
    std::atomic_bool mIsConnect;
    std::uint32_t mRetryMs;

    std::unique_ptr<Channel> mChannel;
    ConnectionCallback mConnectionCallback;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_CONNECTOR_HPP