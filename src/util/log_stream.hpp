#pragma once
#ifndef ASUKA_LOG_STREAM
#define ASUKA_LOG_STREAM


#include <algorithm>

#include "log_buffer.hpp"

namespace Asuka
{

namespace 
{

// integer to string
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

template <typename T>
inline std::size_t integer_to_string(char* buf, T value)
{
    bool negative = value < 0;
    char* p = buf;
    do
    {
        int lp = static_cast<int>(value % 10);
        value /= 10;
        *p++ = zero[lp];
    }
    while (value);

    if (negative)
    {
        *p++ = '-';
    }
    *p = '\0';
    std::reverse(buf, p);

    return p - buf;
}

// hex integer to string
const char digitsHex[] = "0123456789ABCDEF";
inline std::size_t hex_to_string(char* buf, uintptr_t value)
{
    char* p = buf;
    do
    {
        uintptr_t lp = value % 16;
        value /= 16;
        *p++ = digitsHex[lp];
    }
    while (value);
    *p = '\0';
    std::reverse(buf, p);
    return p - buf;
}

} // unamed namespace

const std::size_t kTinySize = 512;
const std::size_t kMaxNumberLen = 32;


class LogStream
{
public:
    using LBuffer = LogBuffer<kTinySize>;

public:
    LogStream() = default;

    // integer
    LogStream& operator<<(bool value);
    LogStream& operator<<(char value);
    LogStream& operator<<(unsigned char value);
    LogStream& operator<<(signed char value);
    LogStream& operator<<(short value);
    LogStream& operator<<(unsigned short value);
    LogStream& operator<<(int value);
    LogStream& operator<<(unsigned int value);
    LogStream& operator<<(long value);
    LogStream& operator<<(unsigned long value);
    LogStream& operator<<(long long value);
    LogStream& operator<<(unsigned long long value);

    // address
    LogStream& operator<<(const void* value);

    // float-point number
    LogStream& operator<<(float value);
    LogStream& operator<<(double value);


    // string
    LogStream& operator<<(const char* value);
    LogStream& operator<<(const unsigned char* value);
    LogStream& operator<<(const std::string& value);
    LogStream& operator<<(const StringView& value);

    LogStream& operator<<(const LBuffer& buffer);

    void append(const char* data, std::size_t len);
    const LBuffer& get_buffer() const;
    void reset_buffer();

private:
    template <typename T>
    void format_integer(T value)
    {
        assert(mBuffer.available() > kMaxNumberLen);    // too long
        std::size_t len = integer_to_string(mBuffer.current(), value);
        mBuffer.add_current(len);
    }

private:
    LBuffer mBuffer;
};

} // namespace Asuka

#endif // ASUKA_LOG_STREAM