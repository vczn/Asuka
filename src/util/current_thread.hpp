// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_CURRENT_THREAD_HPP
#define ASUKA_CURRENT_THREAD_HPP

#include <thread>
#include <sstream>

namespace Asuka
{

// TODO
// I want to show thread tag on log file, but std::thread seem cannot be expanded
// so I will rewrite thread

inline std::thread::id current_thread_id()
{
    thread_local std::thread::id id = std::this_thread::get_id();
    return id;
}

inline std::string current_thread_id_to_string()
{
    thread_local std::string idstr;
    if (!idstr.empty())
        return idstr;

    std::ostringstream ss;
    ss << current_thread_id();
    idstr = ss.str();

    return idstr;
}


} // namespace Asuka

#endif // ASUKA_CURRENT_THREAD_HPP