#include "socket.hpp"

#include <fcntl.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <cerrno>

#include "../util/logger.hpp"

namespace Asuka
{

namespace Net
{

void set_nonblock(int sockfd)
{
    int flags = 0;
    if ((flags = ::fcntl(sockfd, F_GETFL, 0)) == -1)
    {
        LOG_SYSFATAL << "fcntl F_GETFL error";
    }
    flags |= O_NONBLOCK;

    if (::fcntl(sockfd, F_SETFL, flags) == -1)
    {
        LOG_SYSFATAL << "fcntl F_SETFL error";
    }
}

void set_close_on_exec(int sockfd)
{
    int flags = 0;
    if ((flags = ::fcntl(sockfd, F_GETFD, 0)) == -1)
    {
        LOG_SYSFATAL << "fcntl F_GETFD error";
    }
    
    flags |= O_CLOEXEC;
    if (::fcntl(sockfd, F_SETFD, flags) == -1)
    {
        LOG_SYSFATAL << "fcntl F_SETFD error";
    }
}

void set_nonblock_and_close_on_exec(int sockfd)
{
    set_nonblock(sockfd);
    set_close_on_exec(sockfd);
}

int create_nonblock_socket(sa_family_t family)
{
#ifdef SOCK_NONBLOCK
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK, 0);
#else
    int sockfd = ::socket(family, SOCK_STREAM, 0);
    set_nonblock(sockfd);
#endif // SOCK_NONBLOCK

    if (sockfd == -1)
    {
        LOG_SYSFATAL << "socket error";
    }

    return sockfd;
}

void close_sockfd(int fd)
{
    close_fd(fd, "close sockfd error");
}

int get_socket_error(int sockfd)
{
    int optval = 0;
    socklen_t optlen = static_cast<socklen_t>(sizeof(optval));

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) == -1)
    {
        return errno;
    }
    
    return optval;
}

Socket::Socket(int fd) : mSockfd(fd)
{
    set_reuseaddr(1);
}

Socket::~Socket() noexcept
{
    close();
}

int Socket::get_fd() const
{
    return mSockfd;
}

void Socket::bind(const IpPort& localaddr)
{
    const sockaddr* addr = localaddr.get_sockaddr();
    if (::bind(mSockfd, addr,
        static_cast<socklen_t>(sizeof(sockaddr_in6))) == -1)
    {
        LOG_SYSFATAL << "bind error";
    }
}

void Socket::listen()
{
    // SOMAXCONN 
    // add net.core.somaxconn = xxx into /etc/sysctl.conf
    if (::listen(mSockfd, SOMAXCONN) == -1)
    {
        LOG_SYSFATAL << "listen error";
    }
}

int Socket::accept(IpPort& peeraddr)
{
    sockaddr_in6 addr6;
    socklen_t addrlen = static_cast<socklen_t>(sizeof(addr6));
    ::bzero(&addr6, addrlen);

    sockaddr* addr = reinterpret_cast<sockaddr*>(&addr6);
    int connfd = ::accept(mSockfd, addr, &addrlen);

    if (connfd >= 0)
    {
        peeraddr.set_addr(*addr);
    }
    else
    {
        LOG_SYSERROR << "accept error";
    }
    return connfd;
}

void Socket::connect(const IpPort& peeraddr)
{
    const sockaddr* addr = peeraddr.get_sockaddr();
    if (::connect(mSockfd, addr,
        static_cast<socklen_t>(sizeof(sockaddr_in6))) == -1)
    {
        LOG_SYSFATAL << "connect error";
    }
}

void Socket::shutdown_write()
{
    if (::shutdown(mSockfd, SHUT_WR) == -1)
    {
        LOG_SYSERROR << "shutdown write error";
    }
}

void Socket::close()
{
    close_sockfd(mSockfd);
}

void Socket::set_delay(int optval)
{
    if (::setsockopt(mSockfd, IPPROTO_TCP, TCP_NODELAY,
        &optval, static_cast<socklen_t>(sizeof(optval))) == -1)
    {
        LOG_SYSERROR << "setsockopt TCP_NODELAY error";
    }
}

void Socket::set_keep_alive(int optval)
{
    if (::setsockopt(mSockfd, SOL_SOCKET, SO_KEEPALIVE,
        &optval, static_cast<socklen_t>(sizeof(optval))) == -1)
    {
        LOG_SYSERROR << "setsockopt SO_KEEPALIVE error";
    }
}

void Socket::set_reuseaddr(int optval)
{
    if (::setsockopt(mSockfd, SOL_SOCKET, SO_REUSEADDR,
        &optval, static_cast<socklen_t>(sizeof(optval))) == -1)
    {
        LOG_SYSERROR << "setsockopt SO_REUSEADDR error";
    }
}

void Socket::set_reuseport(int optval)
{
#ifdef SO_REUSEPORT
    if (::setsockopt(mSockfd, SOL_SOCKET, SO_REUSEPORT,
        &optval, static_cast<socklen_t>(sizeof(optval))) == -1)
    {
        LOG_SYSERROR << "setsockopt SO_REUSEPORT error";
    }
#else 
    if (optval)
    {
        LOG_SYSERROR << "SO_REUSEPORT is not support";
    }
#endif // SO_REUSEPORT
}

} // namespace Net

} // namespace Asuka