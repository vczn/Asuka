#include "ip_port.hpp"

#include <arpa/inet.h>
#include <netdb.h>

#include <cassert>
#include <cinttypes>
#include <cstring>

#include "../util/logger.hpp"
#include "endian.hpp"

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif // __STDC_FORMAT_MACROS

namespace Asuka
{

namespace Net
{

namespace
{

void check_inet_pton(int err, const char* ip)
{
    if (err == 0)
    {
        LOG_ERROR << ip << " is not a valid net address";
    }
    else if (err == -1)
    {
        LOG_SYSERROR << "inet_pton error";
    }
}

void check_inet_ntop(const char* err)
{
    if (err == nullptr)
    {
        LOG_SYSERROR << "inet_ntop error";
    }
}

} // unamed namespace


void to_ipport(const char* ip, std::uint16_t port, sockaddr_in* addr)
{
    addr->sin_family = AF_INET;
    addr->sin_port = host_to_net16(port);
    int err = ::inet_pton(AF_INET, ip, &addr->sin_addr);

    check_inet_pton(err, ip);
}

void to_ipport(const char* ip, std::uint16_t port, sockaddr_in6* addr)
{
    addr->sin6_family = AF_INET6;
    addr->sin6_port = host_to_net16(port);
    int err = ::inet_pton(AF_INET6, ip, &addr->sin6_addr);

    check_inet_pton(err, ip);
}

void ip_to_string(char* buf, socklen_t len, const sockaddr* addr)
{
    assert(addr->sa_family == AF_INET || addr->sa_family == AF_INET6);
    if (addr->sa_family == AF_INET)
    {
        assert(len > INET_ADDRSTRLEN);
        const struct sockaddr_in* addr4 = 
            reinterpret_cast<const struct sockaddr_in*>(addr);
        check_inet_ntop(::inet_ntop(AF_INET, &addr4->sin_addr, buf, len));
    }
    else if (addr->sa_family == AF_INET6)
    {
        assert(len > INET6_ADDRSTRLEN);
        const struct sockaddr_in6* addr6 =
            reinterpret_cast<const struct sockaddr_in6*>(addr);
        check_inet_ntop(::inet_ntop(AF_INET6, &addr6->sin6_addr, buf, len));
    }
}

void ipport_to_string(char* buf, socklen_t len, const sockaddr* addr)
{
    ip_to_string(buf, len, addr);
    std::size_t endpos = std::strlen(buf);
    assert(len > endpos + 6);

    std::uint16_t port = 0;
    if (addr->sa_family == AF_INET)
    {
        const struct sockaddr_in* addr4 =
            reinterpret_cast<const struct sockaddr_in*>(addr);
        port = net_to_host16(addr4->sin_port);
    }
    else // addr->sa_family == AF_INET6
    {
        const struct sockaddr_in6* addr6 =
            reinterpret_cast<const struct sockaddr_in6*>(addr);
        port = net_to_host16(addr6->sin6_port);
    }

    std::snprintf(buf + endpos, len - endpos, ":" PRIu16"", port);
}

IpPort::IpPort() : mIPv6(false)
{
    ::bzero(&mAddr4, sizeof(mAddr4));
}

IpPort::IpPort(std::uint16_t port, bool ipv6) : mIPv6(ipv6)
{
    if (ipv6)
    {
        ::bzero(&mAddr6, sizeof(mAddr6));
        mAddr6.sin6_family = AF_INET6;
        mAddr6.sin6_addr = in6addr_any;
        mAddr6.sin6_port = host_to_net16(port);
    }
    else // ipv4
    {
        ::bzero(&mAddr4, sizeof(mAddr4));
        mAddr4.sin_family = AF_INET;
        mAddr4.sin_addr.s_addr = host_to_net32(INADDR_ANY);
        mAddr4.sin_port = host_to_net16(port);
    }
}

IpPort::IpPort(const std::string& ip, std::uint16_t port, bool ipv6)
    : IpPort(ip.c_str(), port, ipv6)
{
}

IpPort::IpPort(const char* ip, std::uint16_t port, bool ipv6)
    : mIPv6(ipv6)
{
    if (ipv6)
    {
        ::bzero(&mAddr6, sizeof(mAddr6));
        to_ipport(ip, port, &mAddr6);
    }
    else
    {
        ::bzero(&mAddr4, sizeof(mAddr4));
        to_ipport(ip, port, &mAddr4);
    }
}

IpPort::IpPort(const sockaddr_in& addr)
    : mAddr4(addr), mIPv6(false)
{
}

IpPort::IpPort(const sockaddr_in6 & addr6)
    : mAddr6(addr6), mIPv6(true)
{
}

std::string IpPort::get_ip() const
{
    char buf[64] = "";

    ip_to_string(buf, sizeof(buf), get_sockaddr());

    return std::string{ buf };
}

std::uint16_t IpPort::get_port() const
{
    return host_to_net16(get_port_net_endian());
}

std::string IpPort::get_ipport() const
{
    char buf[64] = "";
    ipport_to_string(buf, sizeof(buf), get_sockaddr());

    return std::string{ buf };
}

std::uint32_t IpPort::get_ip_net_endian() const
{
    assert(get_family() == AF_INET);
    return mAddr4.sin_addr.s_addr;
}

std::uint16_t IpPort::get_port_net_endian() const
{
    return mIPv6 ? mAddr6.sin6_port : mAddr4.sin_port;
}

sa_family_t IpPort::get_family() const
{
    return mIPv6 ? mAddr6.sin6_family : mAddr4.sin_family;
}

const sockaddr * IpPort::get_sockaddr() const
{
    return mIPv6
        ? reinterpret_cast<const struct sockaddr*>(&mAddr6)
        : reinterpret_cast<const struct sockaddr*>(&mAddr4);
}

void IpPort::set_addr(const sockaddr& addr)
{
    if (addr.sa_family == AF_INET)
    {
        mIPv6 = false;
        mAddr4 = *reinterpret_cast<const sockaddr_in*>(&addr);
    }
    else
    {
        mIPv6 = true;
        mAddr6 = *reinterpret_cast<const sockaddr_in6*>(&addr);
    }
}

} // namespace Net

} // namespace Asuka