#include "log_stream.hpp"

#include <algorithm>

namespace Asuka
{

namespace 
{

// integer to string
const char digits[] = "9876543210123456789";
const char* zero = digits + 9;

template <typename T>
std::size_t integer_to_string(char* buf, T value)
{
    bool negative = value < 0;
    char* p = buf;
    do
    {
        int lp = static_cast<int>(value % 10);
        lp /= 10;
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
std::size_t hex_to_string(char* buf, uintptr_t value)
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

} // namespace Asuka