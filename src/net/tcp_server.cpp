#include "tcp_server.hpp"

#include "../util/logger.hpp"
#include "acceptor.hpp"
#include "event_loop.hpp"
#include "tcp_connection.hpp"


namespace Asuka
{

namespace Net
{

// TODO: new -> make_xxx
// needs c++14
TcpServer::TcpServer(EventLoop* loop, const IpPort& listenAddr, 
            std::string name, int reusePort)
    : mLoop(loop),
      mIpPort(listenAddr.get_ipport()),
      mName(std::move(name)),
      mAcceptor(new Acceptor(loop, listenAddr, reusePort)),
      mThreadPool(new EventLoopThreadPool(loop, mName)),
      mConnectionCallback(default_connection_callback),
      mMessageCallback(default_message_callback),
      mNextConnId(1)
{
    mAcceptor->set_newconnection_callback(
        std::bind(&TcpServer::new_connection, this,
        std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
    mLoop->assert_in_loop_thread();
    LOG_TRACE << "TcpServer::~TcpServer [" << mName << "]";

    for (auto& conn : mConnections)
    {
        TcpConnectionPtr cp{ conn.second };
        conn.second.reset();
        cp->get_loop()->run_in_loop(
            std::bind(&TcpConnection::connect_destroy, cp));
    }
}

const std::string& TcpServer::get_ipport() const
{
    return mIpPort;
}

const std::string& TcpServer::get_name() const
{
    return mName;
}

const EventLoop* TcpServer::get_loop() const
{
    return mLoop;
}

void TcpServer::set_thread_number(std::size_t num)
{
    mThreadPool->set_thread_number(num);
}

void TcpServer::set_thread_init_callback(ThreadInitCallback cb)
{
    mThreadInitCallback = std::move(cb);
}

std::shared_ptr<EventLoopThreadPool> TcpServer::get_thread_pool()
{
    return mThreadPool;
}

void TcpServer::set_connection_callback(ConnectionCallback cb)
{
    mConnectionCallback = std::move(cb);
}

void TcpServer::set_message_callback(MessageCallback cb)
{
    mMessageCallback = std::move(cb);
}

void TcpServer::set_write_complete_callback(WriteCompleteCallback cb)
{
    mWriteCompleteCallback = std::move(cb);
}

void TcpServer::start()
{
    if (mStarted == 0)
    {
        mThreadPool->start();
        mLoop->run_in_loop(
            std::bind(&Acceptor::listen, mAcceptor.get()));
    }
}

void TcpServer::new_connection(int sockfd, const IpPort& clientAddr)
{
    mLoop->assert_in_loop_thread();
    EventLoop* loop = mThreadPool->get_next_loop();
    char buf[64];
    snprintf(buf, sizeof(buf), "-%s#%d", mIpPort.c_str(), mNextConnId);
    ++mNextConnId;
    std::string connName = mName + buf;

    LOG_INFO << "TcpServer::new_connection [" << mName
        << "] new connection [" << connName
        << "] from " << clientAddr.get_ipport();

    IpPort localAddr{ get_local_address(sockfd) };
    // FIXME poll with zero timeout to double confirm the new connection
    TcpConnectionPtr conn{ new TcpConnection{loop, 
        connName, sockfd, localAddr, clientAddr} };
    mConnections[connName] = conn;
    conn->set_connection_callback(mConnectionCallback);
    conn->set_message_callback(mMessageCallback);
    conn->set_write_complete_callback(mWriteCompleteCallback);

    // FIXME: unsafe
    conn->set_close_callback(std::bind(&TcpServer::remove_connection, this,
        std::placeholders::_1));

    loop->run_in_loop(std::bind(&TcpConnection::connect_established, conn));
}

void TcpServer::remove_connection(const TcpConnectionPtr& conn)
{
    mLoop->run_in_loop(std::bind(&TcpServer::remove_connection_in_loop, 
        this, conn));
}

void TcpServer::remove_connection_in_loop(const TcpConnectionPtr & conn)
{
    mLoop->assert_in_loop_thread();

    LOG_INFO << "TcpServer::remove_connection_in_loop[" << mName
        << "] - connection " << conn->get_name();

    std::size_t n = mConnections.erase(conn->get_name());
    assert(n == 1);
    (void)n;

    EventLoop* loop = conn->get_loop();
    loop->queue_in_loop(std::bind(&TcpConnection::connect_destroy, conn));
}


} // namespace Net

} // namespace Asuka