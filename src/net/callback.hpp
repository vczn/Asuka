// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_CALLBACK_HPP
#define ASUKA_CALLBACK_HPP

#include <functional>
#include <memory>

namespace Asuka
{

namespace Net
{

class Buffer;
class TcpConnection;

using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using TimerCallback = std::function<void()>;

} // namespace Net

} // namespace Asuka


#endif // ASUKA_CALLBACK_HPP