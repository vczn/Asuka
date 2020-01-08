#pragma once
#ifndef ASUKA_TCP_SERVER_HPP
#define ASUKA_TCP_SERVER_HPP

#include <atomic>
#include <functional>
#include <map>
#include <memory>

#include "../util/noncopyable.hpp"
#include "event_loop_thread_pool.hpp"
#include "tcp_connection.hpp"

namespace Asuka
{

namespace Net
{

class Acceptor;
// class EventLoopThreadPool;

// TcpServer supports single thread and thread pool
class TcpServer : Noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;
public:
    TcpServer(EventLoop* loop, 
              const IpPort& listenAddr, 
              std::string name, 
              int reusePort = 0);
    ~TcpServer();

    const std::string& get_ipport() const;
    const std::string& get_name() const;
    const EventLoop* get_loop() const;

    // always accept a new connection in the mLoop's thread
    // must be called before calls start()
    // @param num
    // - 0 means all I/O in mLoop's thread, no thread will be created, default
    // - 1 means all I/O in another thread
    // - N(>1) means a thread pool with N threads, new connections 
    //         are assigned on a round-robin basis
    void set_thread_number(std::size_t num);
    void set_thread_init_callback(ThreadInitCallback cb);

    std::shared_ptr<EventLoopThreadPool> get_thread_pool();

    void set_connection_callback(ConnectionCallback cb);
    void set_message_callback(MessageCallback cb);
    void set_write_complete_callback(WriteCompleteCallback cb);

    // start the server if it is not listening
    // thread safe
    void start();

private:
    // not thread safe, but in mLoop
    void new_connection(int sockfd, const IpPort& clientAddr);

    // thread safe
    void remove_connection(const TcpConnectionPtr& conn);

    // not thread safe, but in mLoop
    void remove_connection_in_loop(const TcpConnectionPtr& conn);


    using ConnectionMap = std::map<std::string, TcpConnectionPtr>;
private:
    EventLoop* mLoop;
    const std::string mIpPort;
    const std::string mName;

    std::unique_ptr<Acceptor> mAcceptor;

    std::shared_ptr<EventLoopThreadPool> mThreadPool;

    ConnectionCallback mConnectionCallback;
    MessageCallback mMessageCallback;
    WriteCompleteCallback mWriteCompleteCallback;
    
    std::atomic<std::int32_t> mStarted; 
    ThreadInitCallback mThreadInitCallback;

    // always in the loop thread
    int mNextConnId;
    
    ConnectionMap mConnections;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_TCP_SERVER_HPP