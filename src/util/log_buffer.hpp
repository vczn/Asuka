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
    LogBuffer() : mCur(mData)
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
        return mData;
    }

    std::size_t size() const
    {
        return mCur - mData;
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
        mCur = mData;
    }

    void bzero()
    {
        std::memset(mData, 0, N);
    }

    std::string to_string() const
    {
        return std::string(mData, size());
    }

    StringView to_string_view() const
    {
        return StringView{ mData, size() };
    }

private:
    char mData[N];
    char* mCur;
};

} // namespace Asuka

#endif // ASUKA_LOG_BUFFER_HPP