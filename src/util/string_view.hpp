// Part of Asuka utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_STRING_VIEW_HPP
#define ASUKA_STRING_VIEW_HPP

#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include "util.hpp"

namespace Asuka
{

// make wheel with c++11/14
// don't use std::string_view in c++17
// not template


CONSTEXPR14 std::size_t get_length(const char* str)
{
    std::size_t len{};
    while (*str != '\0')
    {
        ++str;
        ++len;
    }
    return len;
}

// A constant contiguous sequence of `char`
// StringView is far more effective
// for constant contiguous sequence than std::string
class StringView
{
public:
    using value_type        = char;
    using size_type         = std::size_t;
    using difference_type   = std::ptrdiff_t;
    using pointer           = char*;
    using const_pointer     = const char*;
    using reference         = char&;
    using const_reference   = const char&;
    using const_iterator    = const char*;

public:
    static constexpr auto npos = static_cast<size_type>(-1);

public:
    constexpr StringView() noexcept
        : mData(), 
          mSize(0)
    {
    }

    constexpr StringView(const StringView& rhs) noexcept = default;
    constexpr StringView& operator=(const StringView& rhs) noexcept = default;

    StringView(const std::string& str)  // extend
        : mData(str.data()),
          mSize(str.size())
    {
    }

    constexpr StringView(const_pointer str, size_type len)
        : mData(str),
          mSize(len)
    {
    }

    CONSTEXPR14 StringView(const_pointer str)
        : mData(str),
          mSize(get_length(str))
    {
        assert(mSize != 0);
        assert(str != nullptr);
    }

    constexpr const_iterator begin() const noexcept
    {
        return mData;
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    constexpr const_iterator end() const noexcept
    {
        return mData + mSize;
    }

    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

    CONSTEXPR14 const_reference operator[](size_type pos) const
    {
        assert(pos < mSize);
        return mData[pos];
    }

    CONSTEXPR14 const_reference at(size_type pos) const
    {
        check_offset(pos);

        return mData[pos];
    }

    constexpr const_reference front() const
    {
        return this->operator[](0);
    }

    constexpr const_reference back() const
    {
        return this->operator[](mSize - 1);
    }

    constexpr const_pointer data() const noexcept
    {
        return mData;
    }

    constexpr size_type max_size() const noexcept
    {
        return npos;
    }

    constexpr bool empty() const noexcept
    {
        return mSize == 0;
    }

    constexpr size_type size() const noexcept
    {
        return mSize;
    }

    constexpr size_type length() const noexcept
    {
        return mSize;
    }

    CONSTEXPR14 void remove_prefix(const size_type count) noexcept
    {
        assert(count < size());
        mData += count;
        mSize -= count;
    }

    CONSTEXPR14 void remove_suffix(const size_type count) noexcept
    {
        assert(count < size());
        mSize -= count;
    }

    CONSTEXPR14 void swap(StringView& rhs) noexcept
    {
        // std::swap isn't constexpr
        StringView tmp{ rhs };
        rhs = *this;
        *this = tmp;
    }
    
    CONSTEXPR14 StringView substr(size_type pos = 0, 
        size_type count = npos) const
    {
        check_offset(pos);
        count = std::min(count, size() - pos);

        return StringView{ mData + pos, count };
    }
    
    size_type copy(char* dest, size_type count, 
        size_type pos = 0) const
    {
        assert(pos < size());
        count = std::min(count, size() - pos);
        std::memcpy(dest, mData + pos, count);

        return count;
    }

    // parameter is StringView in c++ standard

    CONSTEXPR14 int compare(const StringView& rhs) const noexcept
    {
        size_type minSize = std::min(mSize, rhs.size());
        int rc = std::memcmp(mData, rhs.mData, minSize);
        if (rc == 0)
        {
            if (mSize < rhs.mSize)
            {
                rc = -1;
            }
            else if (mSize > rhs.mSize)
            {
                rc = 1;
            }
        }

        return rc;
    }

    CONSTEXPR14 int compare(size_type pos1, size_type count1,
        const StringView& rhs) const 
    {
        return substr(pos1, count1).compare(rhs);
    }

    CONSTEXPR14 int compare(size_type pos1, size_type count1,
        const StringView& rhs, size_type pos2, size_type count2) const
    {
        return substr(pos1, count1).compare(rhs.substr(pos2, count2));
    }

    CONSTEXPR14 int compare(const char* str) const
    {
        return compare(StringView{ str });
    }

    CONSTEXPR14 int compare(size_type pos1, size_type count1,
        const char* str) const
    {
        return substr(pos1, count1).compare(StringView{ str });
    }

    constexpr int compare(size_type pos1, size_type count1,
        const char* str, size_type count2) const 
    {
        return substr(pos1, count1).compare(StringView{ str, count2 });
    }

    constexpr bool starts_with(const StringView& x) const noexcept
    {
        return size() >= x.size() && compare(0, x.size(), x) == 0;
    }

    constexpr bool starts_with(char x) const noexcept
    {
        return starts_with(StringView{ &x, 1 });
    }

    CONSTEXPR14 bool starts_with(const char* x) const
    {
        return starts_with(StringView{ x });
    }

    constexpr bool ends_with(const StringView& x) const noexcept
    {
        return size() >= x.size() && 
            compare(size() - x.size(), x.size(), x) == 0;
    }

    constexpr bool ends_with(char x) const noexcept
    {
        return ends_with(StringView{ &x, 1 });
    }

    constexpr bool ends_with(const char* x) const
    {
        return ends_with(StringView{ x });
    }

    CONSTEXPR14 size_type find(const StringView& sv, 
        size_type pos1 = 0) const noexcept
    {
        if (mSize < sv.mSize || pos1 + sv.mSize > mSize)
        {
            return npos;
        }

        for (size_type i = pos1; i <= size() - sv.size(); ++i)
        {
            if (mData[i] == sv[0])  // matched the first character
            {
                size_type j = 1;
                for (; j < sv.size(); ++j)
                {
                    if (mData[i + j] != sv[j]) // mismatched
                    {
                        break;
                    }
                }
                if (j == sv.size())  // matched all characters
                {
                    return i;
                }
            }
        }
        
        return npos;
    }

    CONSTEXPR14 size_type find(char ch, size_type pos1 = 0) const noexcept
    {
        return find(StringView{ &ch, 1 }, pos1);
    }

    CONSTEXPR14 size_type find(const char* str, 
        size_type pos1, size_type count2) const noexcept
    {
        return find(StringView{ str, count2 }, pos1);
    }

    CONSTEXPR14 size_type find(const char* str, size_type pos1 = 0) const
    {
        return find(StringView{ str }, pos1);
    }

    std::string to_string() const
    {
        return std::string{ mData, mSize };
    }

private:
    CONSTEXPR14 void check_offset(const size_type offset) const
    {
        if (mSize < offset)
        {
            throw std::out_of_range{ "out of range" };
        }
    }

private:
    const char* mData;
    size_type mSize;
};

CONSTEXPR14 bool operator==(const StringView& lhs, const StringView& rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

CONSTEXPR14 bool operator!=(const StringView& lhs, const StringView& rhs) noexcept
{
    return !(lhs == rhs);
}

CONSTEXPR14 bool operator<(const StringView& lhs, const StringView& rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

CONSTEXPR14 bool operator<=(const StringView& lhs, const StringView& rhs) noexcept
{
    return !(rhs < lhs);
}

CONSTEXPR14 bool operator>(const StringView& lhs, const StringView& rhs) noexcept
{
    return rhs < lhs;
}

CONSTEXPR14 bool operator>=(const StringView& lhs, const StringView& rhs) noexcept
{
    return !(lhs < rhs);
}

} // namespace WebServer


#endif // ASUKA_STRING_VIEW_HPP