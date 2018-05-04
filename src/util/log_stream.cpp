#include "log_stream.hpp"

#include <cassert>

namespace Asuka
{

void LogStream::append(const char* data, std::size_t len)
{
    mBuffer.append(data, len);
}

const LogStream::LBuffer& LogStream::get_buffer() const
{
    return mBuffer;
}

void LogStream::reset_buffer()
{
    mBuffer.reset();
}

LogStream& LogStream::operator<<(bool value)
{
    if (value)
    {
        append("true", 4);
    }
    else
    {
        append("false", 5);
    }

    return *this;
}

LogStream& LogStream::operator<<(char value)
{
    append(&value, 1);
    return *this;
}

LogStream& LogStream::operator<<(short value)
{
    *this << (static_cast<int>(value));
    return *this;
}

LogStream& LogStream::operator<<(unsigned short value)
{
    *this << (static_cast<unsigned int>(value));
    return *this;
}

LogStream& LogStream::operator<<(int value)
{
    format_integer(value);
    return *this;
}

LogStream& LogStream::operator<<(unsigned int value)
{
    format_integer(value);
    return *this;
}

LogStream& LogStream::operator<<(long value)
{
    format_integer(value);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long value)
{
    format_integer(value);
    return *this;
}

LogStream& LogStream::operator<<(long long value)
{
    format_integer(value);
    return *this;
}

LogStream& LogStream::operator<<(unsigned long long value)
{
    format_integer(value);
    return *this;
}

LogStream& LogStream::operator<<(const void* value)
{
    uintptr_t ptr = reinterpret_cast<uintptr_t>(value);
    assert(mBuffer.available() > kMaxNumberLen);
    char* buf = mBuffer.current();
    buf[0] = '0';
    buf[1] = 'x';

    size_t len = hex_to_string(buf + 2, ptr);
    mBuffer.add_current(len + 2);

    return *this;
}

LogStream& LogStream::operator<<(float value)
{
    *this << (static_cast<double>(value));
    return *this;
}

LogStream& LogStream::operator<<(double value)
{
    assert(mBuffer.available() > kMaxNumberLen);
    int len = snprintf(mBuffer.current(), kMaxNumberLen,
        "%.12g", value);
    mBuffer.add_current(static_cast<size_t>(len));

    return *this;
}

LogStream& LogStream::operator<<(const char* value)
{
    if (value)
    {
        append(value, std::strlen(value));
    }
    else
    {
        append("nullptr", 7);
    }

    return *this;
}

LogStream& LogStream::operator<<(const unsigned char* value)
{
    return operator<<(reinterpret_cast<const char*>(value));
}

LogStream& LogStream::operator<<(const std::string& value)
{
    append(value.c_str(), value.size());
    return *this;
}

LogStream& LogStream::operator<<(const StringView& value)
{
    append(value.data(), value.size());
    return *this;
}

LogStream& LogStream::operator<<(const LBuffer& buffer)
{
    return operator<<(buffer.to_string_view());
}

} // namespace Asuka