// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_ENDIAN_HPP
#define ASUKA_ENDIAN_HPP

#include <endian.h>

#include <cstdint>

namespace Asuka
{

inline std::uint16_t host_to_net16(std::uint16_t val)
{
    return htobe16(val);
}

inline std::uint32_t host_to_net32(std::uint32_t val)
{
    return htobe32(val);
}

inline std::uint64_t host_to_net64(std::uint64_t val)
{
    return htobe64(val);
}

inline std::uint16_t net_to_host16(std::uint16_t val)
{
    return be16toh(val);
}

inline std::uint32_t net_to_host32(std::uint32_t val)
{
    return be32toh(val);
}

inline std::uint64_t net_to_host64(std::uint64_t val)
{
    return be64toh(val);
}

} // namespace Asuka

#endif // ASUKA_ENDIAN_HPP