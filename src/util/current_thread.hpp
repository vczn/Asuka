// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_CURRENT_THREAD_HPP
#define ASUKA_CURRENT_THREAD_HPP

#include <thread>

namespace Asuka
{

inline std::thread::id current_thread_id()
{
    static thread_local std::thread::id id = std::this_thread::get_id();
    return id;
}

} // namespace Asuka

#endif // ASUKA_CURRENT_THREAD_HPP