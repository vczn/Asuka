// Part of Asuka utility, log internal header
// Copyleft 2018, vczn

#include "log_buffer.hpp"

#pragma once
#ifndef ASUKA_LOG_STREAM
#define ASUKA_LOG_STREAM

namespace Asuka
{

const std::size_t kTinySize = 512;

class LogStream
{
public:
    using LBuffer = LogBuffer<kTinySize>;
public:
    // integer
    LogStream& operator<<(bool value);
    LogStream& operator<<(char value);
    LogStream& operator<<(signed char value);
    LogStream& operator<<(unsigned char value);
    LogStream& operator<<(short value);
    LogStream& operator<<(unsigned short value);
    LogStream& operator<<(int value);
    LogStream& operator<<(unsigned value);
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
    LBuffer mBuffer;
};

} // namespace Asuka

#endif // ASUKA_LOG_STREAM