// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_NONCOPYABLE_HPP
#define ASUKA_NONCOPYABLE_HPP

#include "cxx_version.hpp"

namespace Asuka
{

class Noncopyable 
{
protected:
    Noncopyable() = default;
    ~Noncopyable() = default;

public:
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable& operator=(const Noncopyable&) = delete;
};

} // namespace Asuka


#endif // ASUKA_NONCOPYABLE_HPP