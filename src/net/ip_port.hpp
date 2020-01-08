#pragma once
#ifndef ASUKA_IP_PORT_HPP
#define ASUKA_IP_PORT_HPP

#include <netinet/in.h>
#include <sys/socket.h>

#include <cstdint>
#include <string>

namespace Asuka
{

namespace Net
{

union SockaddrUnion 
{
    struct sockaddr sa;
    struct sockaddr_in in4;
    struct sockaddr_in6 in6;
};

// IP string + 16 bit port# => IPv4 addr
void to_ipport(const char* ip, std::uint16_t port, struct sockaddr_in* addr);

// IP string + 16 bit port# => IPv6 addr
void to_ipport(const char* ip, std::uint16_t port, struct sockaddr_in6* addr);

// sockaddr => IP string
void ip_to_string(char* buf, socklen_t len, const struct sockaddr* addr);

// sockaddr => IP + port string
void ipport_to_string(char* buf, socklen_t len, const struct sockaddr* addr);

class IpPort
{
public:
    IpPort();

    IpPort(std::uint16_t port, bool ipv6 = false);

    IpPort(const std::string& ip, std::uint16_t port, bool ipv6 = false);
    IpPort(const char* ip, std::uint16_t port, bool ipv6 = false);

    IpPort(const sockaddr_in& addr);
    IpPort(const sockaddr_in6& addr6);
    IpPort(const SockaddrUnion& addr);

    IpPort(const IpPort& rhs) = default;
    IpPort& operator=(const IpPort& rhs) = default;

    std::string get_ip() const;
    std::uint16_t get_port() const;
    std::string get_ipport() const;

    std::uint32_t get_ip_net_endian() const;
    std::uint16_t get_port_net_endian() const;

    sa_family_t get_family() const;

    const sockaddr* get_sockaddr() const;
    void set_addr(const sockaddr& addr);

    socklen_t get_address_length();
private:
    SockaddrUnion mSockAddr;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_IP_PORT_HPP