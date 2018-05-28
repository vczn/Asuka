// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_CALLBACK_HPP
#define ASUKA_CALLBACK_HPP

#include <functional>
#include <memory>

#include "../util/time_stamp.hpp"

namespace Asuka
{

namespace Net
{

using TimerCallback = std::function<void()>;


class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using MessageCallback = std::function<void(const TcpConnectionPtr&,
    Buffer& buffer, TimeStamp ts)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback = std::function<void(const TcpConnectionPtr&, std::size_t)>;

void default_connection_callback(const TcpConnectionPtr& conn);
void default_message_callback(const TcpConnectionPtr& conn, 
                    Buffer& buffer, TimeStamp receiveTime);


} // namespace Net

} // namespace Asuka


#endif // ASUKA_CALLBACK_HPP