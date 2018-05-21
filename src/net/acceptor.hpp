// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_ACCEPTOR_HPP
#define ASUKA_ACCEPTOR_HPP

#include "channel.hpp"
#include "ip_port.hpp"
#include "socket.hpp"

namespace Asuka
{

namespace Net
{

class EventLoop;

// `Acceptor` is used by `TcpServer` to 
// listen new connections and accept them
class Acceptor : Noncopyable
{
public:
    using NewConnectionCallback = std::function<void(int, const IpPort&)>;

public:
    Acceptor(EventLoop* loop, const IpPort& listenAddr, int reuseport);
    ~Acceptor();

    void set_newconnection_callback(NewConnectionCallback cb);
    void listen();
    void handle_read();     // accept and call `mConnectionCallback`
private:
    EventLoop* mLoop;
    Socket mSocket;
    Channel mChannel;
    NewConnectionCallback mConnectionCallback;
    int mIdleFd;
    bool mIsListening;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_ACCEPTOR_HPP