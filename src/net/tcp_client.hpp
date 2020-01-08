#pragma once
#ifndef ASUKA_TCP_CLIENT_HPP
#define ASUKA_TCP_CLIENT_HPP

#include <atomic>
#include <memory>
#include <mutex>

#include "../util/noncopyable.hpp"
#include "tcp_connection.hpp"

namespace Asuka
{

namespace Net
{

class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;

class TcpClient : Noncopyable
{
public:
    TcpClient(EventLoop* loop, const IpPort& serverAddr, std::string name);
    ~TcpClient();

    void connect();
    void disconnect();
    void stop();

    TcpConnectionPtr get_connection() const;
    const EventLoop* get_loop() const;
    const std::string& get_name() const;

    bool retry() const;
    void enable_retry();

    void set_connection_callback(ConnectionCallback cb);
    void set_message_callback(MessageCallback cb);
    void set_write_complete_callback(WriteCompleteCallback cb);

private:
    // not thread safe, but in loop
    void new_connection(int sockfd);

    // not thread safe, buf in loop
    void remove_connection(const TcpConnectionPtr& conn);

private:
    EventLoop* mLoop;
    std::shared_ptr<Connector> mConnector;
    const std::string mName;

    ConnectionCallback mConnectionCallback;
    MessageCallback mMessageCallback;
    WriteCompleteCallback mWriteCompleteCallback;

    std::atomic_bool mRetry;
    std::atomic_bool mIsConnect;

    // always in the loop thread
    int mNextConnId;
    mutable std::mutex mMutex;
    TcpConnectionPtr mConnection;   // guarded by mMutex
};


} // namespace Net

} // namespace Asuka

#endif // ASUKA_TCP_CLIENT_HPP