// Part of Asuka utility, log internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_LOG_BUFFER_HPP
#define ASUKA_LOG_BUFFER_HPP

#include <cstdint>
#include <string>

#include "string_view.hpp"

namespace Asuka
{

template <std::size_t N>
class LogBuffer
{
public:
    LogBuffer() : mCur(mBuffer)
    {
    }

    ~LogBuffer()
    {
    }

    void append(const char* buf, std::size_t len)
    {
        if (available() > len)
        {
            ::memcpy(mCur, buf, len);
            mCur += len;
        }
    }

    const char* data() const
    {
        return mBuffer;
    }

    std::size_t size() const
    {
        return mCur - mBuffer;
    }

    // write to buffer directly
    char* current()
    {
        return mCur;
    }

    std::size_t available() const
    {
        return N - size();
    }

    void add_current(std::size_t len)
    {
        mCur += len;
    }

    void reset()
    {
        mCur = mBuffer;
    }

    void bzero()
    {
        ::bzero(mBuffer, N);
    }

    std::string to_string() const
    {
        return std::string(mBuffer, size());
    }

    StringView to_string_view() const
    {
        return StringView{ mBuffer, size() };
    }

private:
    char mBuffer[N];
    char* mCur;
};

} // namespace Asuka

#endif // ASUKA_LOG_BUFFER_HPP