// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_SOCKET_HPP
#define ASUKA_SOCKET_HPP

#include "ip_port.hpp"

namespace Asuka
{

namespace Net
{

void set_nonblock(int sockfd);
void set_close_on_exec(int sockfd);
void set_nonblock_and_close_on_exec(int sockfd);
int create_nonblock_socket(sa_family_t family);
void close_sockfd(int fd);
int get_socket_error(int sockfd);


class Socket
{
public:
    explicit Socket(int fd);
    ~Socket() noexcept;

    int get_fd() const;

    void bind(const IpPort& localaddr);
    void listen();
    int accept(IpPort& peeraddr);
    void connect(const IpPort& peeraddr);

    void shutdown_write();
    void close();

    // `optval`: 1 is on, 0 is off
    // Disable Negle algorithm of tcp if the option is on
    // This is used for application which generate small packets,
    // such as telnet, rlogin, etc
    void set_no_delay(int optval);

    // `optval`: 1 is on, 0 is off
    void set_keep_alive(int optval);

    // `optval`: 1 is on, 0 is off
    void set_reuseaddr(int optval);

    // `optval`: 1 is on, 0 is off
    void set_reuseport(int optval);
private:
    const int mSockfd;
};


} // namespace Net

} // namespace Asuka

#endif // ASUKA_SOCKET_HPP