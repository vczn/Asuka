#include "acceptor.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cassert>

#include "../util/logger.hpp"
#include "event_loop.hpp"

namespace Asuka
{

namespace Net
{

Acceptor::Acceptor(EventLoop* loop, const IpPort& listenAddr, int reuseport)
    : mLoop(loop),
      mSocket(create_nonblock_socket(listenAddr.get_family())),
      mChannel(loop, mSocket.get_fd()),
      mIdleFd(::open("/dev/null", O_RDONLY | O_CLOEXEC)),
      mIsListening(false)
{
    assert(mIdleFd >= 0);
    mSocket.set_reuseaddr(1);
    mSocket.set_reuseport(reuseport);
    mSocket.bind(listenAddr);
    mChannel.set_read_callback(std::bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor()
{
    mChannel.disable_all();
    mChannel.remove();
    close_sockfd(mIdleFd);
}

void Acceptor::set_newconnection_callback(NewConnectionCallback cb)
{
    mConnectionCallback = std::move(cb);
}

void Acceptor::listen()
{
    mLoop->assert_in_loop_thread();
    mSocket.listen();
    mIsListening = true;
    mChannel.enable_read();
}

void Acceptor::handle_read()
{
    mLoop->assert_in_loop_thread();
    IpPort clientAddr;
    int connfd = mSocket.accept(clientAddr);
    if (connfd >= 0)
    {
        LOG_TRACE << "accept from: " << clientAddr.get_ipport();
        if (mConnectionCallback)
        {
            mConnectionCallback(connfd, clientAddr);
        }
        else
        {
            close_sockfd(connfd);    // discard
        }
    }
    else
    {
        LOG_SYSERROR << "accept error";

        // the number of opening file descriptors has been reached
        if (errno == EMFILE)
        {
            close_sockfd(mIdleFd);
            mIdleFd = ::accept(mSocket.get_fd(), nullptr, nullptr);
            close_sockfd(mIdleFd);
            mIdleFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}

} // namespace Net

} // namespace Asuka