#include "tcp_client.hpp"

#include "../util/logger.hpp"
#include "connector.hpp"
#include "event_loop.hpp"
#include "socket.hpp"

namespace Asuka
{

namespace Net
{


namespace 
{

void gremove_connection(EventLoop* loop, const TcpConnectionPtr& conn)
{
    loop->queue_in_loop(std::bind(&TcpConnection::connect_destroy, conn));
}

//void gremove_connector(const ConnectorPtr& connector)
//{
//    
//}

} // unnamed namespace

TcpClient::TcpClient(EventLoop* loop, const IpPort& serverAddr, std::string name)
    : mLoop(check_not_nullptr(loop)), 
      mConnector(new Connector{loop, serverAddr}),
      mName(std::move(name)),
      mConnectionCallback(default_connection_callback),
      mMessageCallback(default_message_callback),
      mRetry(false),
      mIsConnect(false),
      mNextConnId(1),
      mMutex()
{
    mConnector->set_connection_callback(
        [this](int sockfd) { this->new_connection(sockfd); });
    LOG_INFO << "TcpClient::TcpClient[" << mName
        << "] - connector " << mConnector.get();
}

TcpClient::~TcpClient()
{
    LOG_INFO << "TcpClient::~TcpClient[" << mName
        << "] - connector " << mConnector.get();
    TcpConnectionPtr conn;
    bool isUnique = false;

    {
        std::lock_guard<std::mutex> lock{ mMutex };
        isUnique = mConnection.use_count() == 1;
        conn = mConnection;
    }

    if (conn)
    {
        assert(mLoop == conn->get_loop());
        CloseCallback cb = std::bind(gremove_connection, mLoop, 
            std::placeholders::_1);
        mLoop->run_in_loop(
            std::bind(&TcpConnection::set_close_callback, conn, cb));

        if (isUnique)
        {
            conn->force_close();
        }
    }
    else
    {
        if (mConnector)
        {
            mConnector->stop();
            //mLoop->run_after(1, std::bind(gremove_connector, mConnector));
        }
    }
}

void TcpClient::connect()
{
    if (!mIsConnect)
    {
        LOG_INFO << "TcpClient::connect[" << mName << "] - connect to "
            << mConnector->get_server_address().get_ipport();
        mIsConnect = true;
        mConnector->start();
    }
}

void TcpClient::disconnect()
{
    
    if (mIsConnect)
    {
        mIsConnect = false;
        std::lock_guard<std::mutex> lock{ mMutex };
        if (mConnection)
        {
            mConnection->shutdown();
        }
    }
}

void TcpClient::stop()
{
    mIsConnect = false;
    mConnector->stop();
}

TcpConnectionPtr TcpClient::get_connection() const
{
    std::lock_guard<std::mutex> lock{ mMutex };
    return mConnection;
}

const EventLoop* TcpClient::get_loop() const
{
    return mLoop;
}

const std::string& TcpClient::get_name() const
{
    return mName;
}

bool TcpClient::retry() const
{
    return mRetry;
}

void TcpClient::enable_retry()
{
    mRetry = true;
}

void TcpClient::set_connection_callback(ConnectionCallback cb)
{
    mConnectionCallback = std::move(cb);
}

void TcpClient::set_message_callback(MessageCallback cb)
{
    mMessageCallback = std::move(cb);
}

void TcpClient::set_write_complete_callback(WriteCompleteCallback cb)
{
    mWriteCompleteCallback = std::move(cb);
}

void TcpClient::new_connection(int sockfd)
{
    mLoop->assert_in_loop_thread();
    IpPort serverAddr{ get_peer_address(sockfd) };
    IpPort clientAddr{ get_local_address(sockfd) };

    char buf[32];
    snprintf(buf, sizeof(buf), ":%s#%d", serverAddr.get_ipport().c_str(), mNextConnId);
    ++mNextConnId;
    std::string connName = mName + buf;

    TcpConnectionPtr conn{ new TcpConnection{ mLoop, 
        connName, sockfd, clientAddr, serverAddr } };

    conn->set_connection_callback(mConnectionCallback);
    conn->set_message_callback(mMessageCallback);
    conn->set_write_complete_callback(mWriteCompleteCallback);
    conn->set_close_callback(std::bind(&TcpClient::remove_connection,
        this, std::placeholders::_1));  // FIXME: unsafe
    {
        std::lock_guard<std::mutex> lock{ mMutex };
        mConnection = conn;
    }

    conn->connect_established();
}

void TcpClient::remove_connection(const TcpConnectionPtr& conn)
{
    mLoop->assert_in_loop_thread();
    assert(mLoop == conn->get_loop());

    {
        std::lock_guard<std::mutex> lock{ mMutex };
        assert(mConnection == conn);
        mConnection.reset();
    }

    mLoop->queue_in_loop(std::bind(&TcpConnection::connect_destroy, conn));
    if (mRetry && mIsConnect)
    {
        LOG_INFO << "TcpConnection::connect[" << mName << "] - reconnecting to"
            << mConnector->get_server_address().get_ipport();
        mConnector->restart();
    }
}

} // namespace Asuka

} // namespace Asuka