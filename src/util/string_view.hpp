#pragma once
#ifndef ASUKA_STRING_VIEW_HPP
#define ASUKA_STRING_VIEW_HPP

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>

#include "util.hpp"

namespace Asuka
{

template <typename CharT>
struct StringViewIterator
{
    using iterator_category = std::random_access_iterator_tag;
    using value_type        = CharT;
    using pointer           = const CharT*;
    using reference         = const CharT&;
    using difference_type   = std::ptrdiff_t;
    using Self              = StringViewIterator<CharT>;


#ifndef NDEBUG
    constexpr StringViewIterator() noexcept
        : ptr(nullptr), size(0), index(0) 
    {
    }

    constexpr StringViewIterator(const pointer p, const size_t s, const size_t idx) noexcept
        : ptr(p), size(s), index(idx)
    {
    }

    constexpr reference operator*() const noexcept
    {
        assert(ptr != nullptr);
        assert(index < size);
        return ptr[index];
    }

    constexpr pointer operator->() const noexcept
    {
        assert(ptr != nullptr);
        assert(index < size);
        return ptr;
    }

    constexpr Self& operator++() noexcept  // pre
    {
        assert(ptr != nullptr);
        assert(index < size);
        ++index;
        return *this;
    }

    constexpr Self operator++(int) noexcept // post
    {
        Self tmp{ *this };
        ++*this;
        return tmp;
    }

    constexpr Self& operator--() noexcept // pre
    {
        assert(ptr != nullptr);
        assert(index != 0);
        --index;
        return *this;
    }

    constexpr Self operator--(int) noexcept // post
    {
        Self tmp{ *this };
        --*this;
        return tmp;
    }

    constexpr Self& operator+=(const difference_type off) noexcept
    {
        assert(ptr != nullptr);
        if (off < 0)
        {
            assert(index >= static_cast<size_t>(-off));
        }
        else if (off > 0)
        {
            assert(index + off <= size);
        }

        index += off;
        return *this;
    }

    constexpr Self operator+(const difference_type off) const noexcept
    {
        Self tmp{ *this };
        tmp += off;
        return tmp;
    }

    constexpr Self operator-=(const difference_type off) noexcept
    {
        assert(ptr != nullptr);
        if (off > 0)
        {
            assert(index >= static_cast<size_t>(off));
        }
        else if (off < 0)
        {
            assert(index + static_cast<size_t>(-off) <= size);
        }

        index -= off;
        return *this;
    }

    constexpr Self operator-(const difference_type off) const noexcept
    {
        Self tmp{ *this };
        tmp -= off;
        return tmp;
    }

    constexpr difference_type operator-(const Self& rhs) const noexcept
    {
        assert(ptr == rhs.ptr && size == rhs.size);
        return index - rhs.index;
    }

    constexpr reference operator[](const difference_type off) const noexcept
    {
        return *(*this + off);
    }

    constexpr bool operator==(const Self& rhs) const noexcept
    {
        assert(ptr == rhs.ptr && size == rhs.size);
        return index == rhs.index;
    }

    constexpr bool operator<(const Self& rhs) const noexcept
    {
        assert(ptr == rhs.ptr && size == rhs.size);
        return index < rhs.index;
    }

    pointer ptr;
    size_t size;
    size_t index;

#else 
    constexpr StringViewIterator() : ptr(nullptr) {}
    constexpr StringViewIterator(const CharT* p) : ptr(p) {}

    constexpr reference operator*() const noexcept
    {
        return *ptr;
    }

    constexpr pointer operator->() const noexcept
    {
        return std::pointer_traits<pointer>::pointer_to(**this);
    }

    constexpr Self& operator++() noexcept
    {
        ++ptr;
        return *this;
    }

    constexpr Self operator++(int) const noexcept
    {
        Self tmp{ *this };
        ++*this;
        return tmp;
    }

    constexpr Self& operator--() noexcept
    {
        --ptr;
        return *this;
    }

    constexpr Self operator--(int) const noexcept
    {
        Self tmp{ *this };
        --*this;
        return tmp;
    }

    constexpr Self& operator+=(const difference_type off) noexcept
    {
        ptr += off;
        return *this;
    }

    constexpr Self operator+(const difference_type off) const noexcept
    {
        Self tmp{ *this };
        tmp += off;
        return tmp;
    }

    constexpr Self& operator-=(const difference_type off) noexcept
    {
        ptr -= off;
        return *this;
    }

    constexpr Self operator-(const difference_type off) const noexcept
    {
        Self tmp{ *this };
        tmp -= off;
        return tmp;
    }

    constexpr difference_type operator-(const Self& rhs) const noexcept
    {
        return ptr - rhs.ptr;
    }

    constexpr reference operator[](const difference_type off) const noexcept
    {
        return *(*this + off);
    }

    constexpr bool operator==(const Self& rhs) const noexcept
    {
        return ptr == rhs.ptr;
    }

    constexpr bool operator<(const Self& rhs) const noexcept
    {
        return ptr < rhs.ptr;
    }

    pointer ptr;
#endif // NDEBUG

    constexpr bool operator!=(const Self& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    constexpr bool operator>(const Self& rhs) const noexcept
    {
        return rhs < *this;
    }

    constexpr bool operator<=(const Self& rhs) const noexcept
    {
        return !(rhs < *this);
    }

    constexpr bool operator>=(const Self& rhs) const noexcept
    {
        return !(*this < rhs);
    }
};

template <typename CharT>
constexpr StringViewIterator<CharT> operator+(
    const typename StringViewIterator<CharT>::difference_type off,
    StringViewIterator<CharT> rhs) noexcept
{
    rhs += off;
    return rhs;
}

template <typename CharT, 
    typename Traits = std::char_traits<CharT>>
class BasicStringView
{
public:
    using value_type                = CharT;
    using traits_type               = Traits;
    using pointer                   = CharT*;
    using const_pointer             = const CharT*;
    using reference                 = CharT&;
    using const_reference           = const CharT&;
    using const_iterator            = StringViewIterator<CharT>;
    using iterator                  = const_iterator;
    using const_reverse_iterator    = std::reverse_iterator<const_iterator>;
    using reverse_iterator          = std::reverse_iterator<iterator>;
    using size_type                 = std::size_t;
    using difference_type           = std::ptrdiff_t;

public:
    static constexpr size_type npos = static_cast<size_type>(-1);

    constexpr BasicStringView() noexcept
        : mData(nullptr), mSize(0)
    {
    }

    constexpr BasicStringView(const BasicStringView& rhs) noexcept = default;

    constexpr BasicStringView(const CharT* s, size_type count)
        : mData(s), mSize(count)
    {
    }

    constexpr BasicStringView& operator=(const BasicStringView& rhs) noexcept = default;

    // use constexpr version instead of char_traits::length
    // std::char_traits constexpr length need C++17
    // C++14 should be able to implement it
    // TODO char_traits
    CONSTEXPR14 BasicStringView(const CharT* s)
        : mData(s), mSize(get_string_length(s))    
    {
    }

    BasicStringView(const std::basic_string<CharT>& str)
        : mData(str.data()),
          mSize(str.size())
    {
    }

    constexpr const_iterator begin() const noexcept
    {
#ifndef NDEBUG
        return const_iterator{ mData, mSize, 0 };
#else 
        return const_iterator{ mData };
#endif  // NDEBUG
    }

    constexpr const_iterator cbegin() const noexcept
    {
        return begin();
    }

    constexpr const_iterator end() const noexcept
    {
#ifndef NDEBUG
        return const_iterator{ mData, mSize, mSize };
#else 
        return const_iterator{ mData + mSize };
#endif  // _DEBUG
    }

    constexpr const_iterator cend() const noexcept
    {
        return end();
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator{ end() };
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return rbegin();
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator{ begin() };
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
        return rend();
    }

    CONSTEXPR14 const_reference operator[](size_type pos) const
    {
        assert(pos < mSize);
        return mData[pos];
    }

    CONSTEXPR14 const_reference at(size_type pos) const
    {
        check_postion_eq(pos);

        return mData[pos];
    }

    constexpr const_reference front() const
    {
        return operator[](0);
    }

    constexpr const_reference back() const
    {
        return operator[](mSize - 1);
    }

    constexpr const_pointer data() const noexcept
    {
        return mData;
    }

    constexpr size_type size() const noexcept
    {
        return mSize;
    }

    constexpr size_type length() const noexcept
    {
        return mSize;
    }

    constexpr size_type max_size() const noexcept
    {
        return static_cast<size_type>(-1) / sizeof(CharT);
    }

    constexpr bool empty() const noexcept
    {
        return mSize == 0;
    }

    CONSTEXPR14 void remove_prefix(size_type n)
    {
        assert(n <= mSize);
        mData += n;
        mSize -= n;
    }

    CONSTEXPR14 void remove_suffix(size_type n)
    {
        assert(n < mSize);
        mSize -= n;
    }

    CONSTEXPR14 void swap(BasicStringView& rhs) noexcept
    {
        if (this != &rhs)
        {
            BasicStringView tmp{ rhs };
            rhs = *this;
            *this = tmp;
        }
    }

    size_type copy(CharT* dst, size_type count, size_type pos = 0) const
    {
        check_position(pos);
        count = std::min(count, mSize - pos);
        Traits::copy(dst, mData + pos, count);
        return count;
    }

    CONSTEXPR14 BasicStringView substr(size_type pos = 0, 
        size_type count = npos) const
    {
        check_position(pos);
        count = std::min(count, mSize - pos);
        return BasicStringView{ mData + pos, count };
    }

    CONSTEXPR14 int compare(BasicStringView rhs) const noexcept
    {
        return compare(mData, mSize, rhs.mData, rhs.mSize);
    }

    CONSTEXPR14 int compare(size_type pos1, size_type count1, 
        BasicStringView rhs) const
    {
        return substr(pos1, count1).compare(rhs);
    }

    CONSTEXPR14 int compare(size_type pos1, size_type count1, 
        BasicStringView rhs, size_type pos2, size_type count2) const
    {
        return substr(pos1, count1).compare(rhs.substr(pos2, count2));
    }

    CONSTEXPR14 int compare(const CharT* str) const
    {
        return compare(BasicStringView{ str });
    }

    CONSTEXPR14 int compare(size_type pos1, size_type count1,
        const CharT* str) const
    {
        return substr(pos1, count1).compare(str);
    }

    CONSTEXPR14 int compare(size_type pos1, size_type count1,
        const CharT* str, size_type count2) const
    {
        return substr(pos1, count1).compare(BasicStringView{ str, count2 });
    }

    CONSTEXPR14 bool starts_with(BasicStringView rhs) const noexcept
    {
        return mSize >= rhs.mSize && compare(0, rhs.mSize, rhs) == 0;
    }

    CONSTEXPR14 bool starts_with(CharT ch) const noexcept
    {
        return starts_with(BasicStringView{ &ch, 1 });
    }

    CONSTEXPR14 bool starts_with(const CharT* str) const noexcept
    {
        return starts_with(BasicStringView{ str });
    }

    CONSTEXPR14 bool ends_with(BasicStringView rhs) const noexcept
    {
        return mSize >= rhs.mSize &&
            compare(mSize - rhs.mSize, rhs.mSize, rhs) == 0;
    }

    CONSTEXPR14 bool ends_with(CharT ch) const noexcept
    {
        return ends_with(BasicStringView{ &ch, 1 });
    }

    CONSTEXPR14 bool ends_with(const CharT* str) const noexcept
    {
        return ends_with(BasicStringView{ str });
    }

    CONSTEXPR14 size_type find(BasicStringView rhs, size_type pos1 = 0) const noexcept
    {
        if (mSize < rhs.mSize || pos1 + rhs.mSize > mSize)
        {
            return npos;
        }

        if (rhs.empty())    // always matches
        {
            return std::min(pos1, mSize);
        }

        for (size_type i = pos1; i <= mSize - rhs.mSize; ++i)
        {
            if (mData[i] == rhs[0])     // matched the first element
            {
                size_type j = 1;
                for (j = 1; j < rhs.mSize; ++j)
                {
                    if (mData[i + j] != rhs[j]) // mismatched
                    {
                        break;
                    }
                }

                if (j == rhs.mSize)
                {
                    return i;   // matched
                }
            }
        }

        return npos;
    }

    CONSTEXPR14 size_type find(CharT ch, size_type pos1 = 0) const noexcept
    {
        for (size_type i = pos1; i < mSize; ++i)
        {
            if (mData[i] == ch)
            {
                return i;   // matched
            }
        }

        return npos;
    }

    CONSTEXPR14 size_type find(const CharT* str, size_type pos1, 
        size_type count2) const noexcept
    {
        return find(BasicStringView{ str, count2 }, pos1);
    }

    CONSTEXPR14 size_type find(const CharT* str, size_type pos1) const noexcept
    {
        return find(BasicStringView{ str }, pos1);
    }

    CONSTEXPR14 size_type rfind(BasicStringView rhs, 
        size_type pos1 = npos) const noexcept
    {
        if (rhs.mSize == 0)     // always matches
        {
            return std::min(pos1, mSize);
        }

        if (rhs.mSize > mSize)
        {
            return npos;
        }

        for (size_type i = std::min(pos1, mSize - rhs.mSize); i >= 0; --i)
        {
            if (mData[i] == rhs[0])     // matched the first element
            {
                size_type j = 1;
                for (j = 1; j < rhs.mSize; ++j)
                {
                    if (mData[i + j] != rhs[j])
                    {
                        break;
                    }
                }

                if (j == rhs.mSize)
                {
                    return i;
                }
            }
        }

        return npos;
    }

    CONSTEXPR14 size_type rfind(CharT ch, size_type pos1 = npos) const noexcept
    {
        if (mSize == 0)
        {
            return npos;
        }

        for (size_type i = std::min(pos1, mSize - 1); i >= 0; --i)
        {
            if (mData[i] == ch)
            {
                return i;   // matched
            }
        }

        return npos;
    }

    CONSTEXPR14 size_type rfind(const CharT* str, size_type pos1, 
        size_type count2) const
    {
        return rfind(BasicStringView{ str, count2 }, pos1);
    }

    CONSTEXPR14 size_type rfind(const CharT* str, size_type pos1 = npos) const
    {
        return rfind(BasicStringView{ str }, pos1);
    }

    std::string to_string() const 
    {
        return std::string{ mData, mSize };
    }

private:
    static CONSTEXPR14 size_type get_string_length(const_pointer str) noexcept
    {
        size_type len = Traits::length(str);
        return len;
    }

    static CONSTEXPR14 int compare(const_pointer lhs, const size_type lsize,
        const_pointer rhs, const size_type rsize) noexcept
    {
        const int ans = Traits::compare(lhs, rhs, std::min(lsize, rsize));
        if (ans != 0)
        {
            return ans;
        }

        if (lsize < rsize)
        {
            return -1;
        }

        if (lsize > rsize)
        {
            return 1;
        }

        return 0;
    }

    constexpr void check_position(const size_type pos) const
    {
        if (pos > mSize)
        {
            Xrange();
        }
    }

    constexpr void check_postion_eq(const size_type pos) const
    {
        if (pos >= mSize)
        {
            Xrange();
        }
    }

    static void Xrange()
    {
        throw std::out_of_range("invalid string_base_view<CharT> out of range");
    }

    const_pointer mData;
    size_type mSize;
};

template <typename CharT, typename Traits>
constexpr bool operator==(BasicStringView<CharT, Traits> lhs,
    BasicStringView<CharT> rhs) noexcept
{
    return lhs.size() == rhs.size() &&
        std::equal(lhs.begin(), lhs.end(), rhs.begin());
}


template <typename CharT, typename Traits>
constexpr bool operator!=(BasicStringView<CharT, Traits> lhs,
    BasicStringView<CharT> rhs) noexcept
{
    return !(lhs == rhs);
}

template <typename CharT, typename Traits>
constexpr bool operator<(BasicStringView<CharT, Traits> lhs,
    BasicStringView<CharT> rhs) noexcept
{
    return stdlexicographical_compare(lhs.begin(), lhs.end(), 
        rhs.begin(), rhs.end());
}

template <typename CharT, typename Traits>
constexpr bool operator>(BasicStringView<CharT, Traits> lhs,
    BasicStringView<CharT> rhs) noexcept
{
    return rhs < lhs;
}

template <typename CharT, typename Traits>
constexpr bool operator<=(BasicStringView<CharT, Traits> lhs,
    BasicStringView<CharT> rhs) noexcept
{
    return !(rhs < lhs);
}

template <typename CharT, typename Traits>
constexpr bool operator>=(BasicStringView<CharT, Traits> lhs,
    BasicStringView<CharT> rhs) noexcept
{
    return !(lhs > rhs);
}

template <typename CharT, typename Traits>
std::basic_ostream<CharT, Traits>& operator<<(
    std::basic_ostream<CharT, Traits>& os,
    BasicStringView<CharT, Traits> sv)
{
    os << sv.data();
    return os;
}

using StringView    = BasicStringView<char>;
using WStringView   = BasicStringView<wchar_t>;
using U16StringView = BasicStringView<char16_t>;
using U32StringView = BasicStringView<char32_t>;

} // namespace Asuka

#endif // ASUKA_STRING_VIEW_HPP