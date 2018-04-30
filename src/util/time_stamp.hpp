// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_TIME_STAMP_HPP
#define ASUKA_TIME_STAMP_HPP

#include <cstdint>

#include "duration.hpp"

namespace Asuka
{

// int64_t represend a timestamp since epoch
class TimeStamp
{
public:
    TimeStamp();
    explicit TimeStamp(std::int64_t us);

private:
    std::int64_t mUs;
};

} // namespace Asuka


#endif // ASUKA_TIME_STAMP_HPP
