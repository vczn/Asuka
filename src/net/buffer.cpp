#include "buffer.hpp"

#include <sys/uio.h>

#include <algorithm>
#include <cassert>
#include <cerrno>
#include <utility>

#include "endian.hpp"

namespace Asuka
{

namespace Net
{ 

const char Buffer::kCRLF[] = "\r\n";
const std::size_t Buffer::kInitialSize = 1024;
const std::size_t Buffer::kPrependSize = 8;


Buffer::Buffer(std::size_t initSize)
    : mBuffer(initSize + kPrependSize),
      mReaderIndex(kPrependSize),
      mWriterIndex(kPrependSize)
{
}

Buffer::Buffer(const Buffer& rhs)
    : mBuffer(rhs.mBuffer),
      mReaderIndex(rhs.mReaderIndex),
      mWriterIndex(rhs.mWriterIndex)
{
}

Buffer::Buffer(Buffer&& rhs) noexcept
    : mBuffer(std::move(rhs.mBuffer)),
      mReaderIndex(rhs.mReaderIndex),
      mWriterIndex(rhs.mWriterIndex)
{
    rhs.mReaderIndex = 0;
    rhs.mWriterIndex = 0;
}

void Buffer::swap(Buffer& rhs) noexcept
{
    mBuffer.swap(rhs.mBuffer);
    std::swap(mReaderIndex, rhs.mReaderIndex);
    std::swap(mWriterIndex, rhs.mWriterIndex);
}

Buffer& Buffer::operator=(const Buffer& rhs)
{
    if (this != &rhs)
    {
        Buffer tmp{ rhs };
        this->swap(tmp);
    }

    return *this;
}

Buffer& Buffer::operator=(Buffer&& rhs) noexcept
{
    if (this != &rhs)
    {
        Buffer tmp = std::move(rhs);
        this->swap(tmp);
    }

    return *this;
}

std::size_t Buffer::readable_bytes() const
{
    return mWriterIndex - mReaderIndex;
}

std::size_t Buffer::writable_bytes() const
{
    return mBuffer.size() - mWriterIndex;
}

std::size_t Buffer::prepend_bytes() const
{
    return mReaderIndex;
}

const char* Buffer::peek() const
{
    return mBuffer.data() + mReaderIndex;
}

const char* Buffer::find_crlf() const
{
    const char* crlf = std::search(read_begin(), write_begin(), kCRLF, kCRLF + 2);
    return crlf == write_begin() ? nullptr : crlf;
}

const char* Buffer::find_crlf(const char* start) const
{
    assert(start >= read_begin());
    assert(start < write_begin());
    const char* crlf = std::search(start, write_begin(), kCRLF, kCRLF + 2);
    return crlf == write_begin() ? nullptr : crlf;
}

const char* Buffer::find_eol() const
{
    const char* eol = std::find(read_begin(), write_begin(), '\n');
    return eol == write_begin() ? nullptr : eol;
}

const char* Buffer::find_eol(const char* start) const
{
    assert(start >= read_begin());
    assert(start < write_begin());
    const char* eol = std::find(start, write_begin(), '\n');
    return eol == write_begin() ? nullptr : eol;
}

char* Buffer::read_begin()
{
    return const_cast<char*>(
        static_cast<const Buffer&>(*this).read_begin());
}

const char* Buffer::read_begin() const
{
    return mBuffer.data() + mReaderIndex;
}

char* Buffer::write_begin()
{
    return const_cast<char*>(
        static_cast<const Buffer&>(*this).write_begin());
}

const char* Buffer::write_begin() const
{
    return mBuffer.data() + mWriterIndex;
}

char* Buffer::begin()
{
    return &*mBuffer.begin();
}

const char* Buffer::begin() const
{
    return &*mBuffer.begin();
}

char* Buffer::end()
{
    return &*mBuffer.end();
}

const char* Buffer::end() const
{
    return &*mBuffer.end();
}

std::size_t Buffer::size() const
{
    return mBuffer.size();
}

std::size_t Buffer::capacity() const
{
    return mBuffer.capacity();
}

void Buffer::reserve(std::size_t sizeBuf)
{
    mBuffer.reserve(sizeBuf);
}

void Buffer::shrink_to_fit()
{
    mBuffer.shrink_to_fit();
}

void Buffer::retrieve(std::size_t len)
{
    assert(len <= readable_bytes());
    if (len < readable_bytes())
    {
        mReaderIndex += len;
    }
    else        // len == readable_bytes(), reset
    {
        retrieve_all();
    }
}

void Buffer::retrieve_all()
{
    mReaderIndex = kPrependSize;
    mWriterIndex = kPrependSize;
}

void Buffer::retrieve_util(const char* end)
{
    assert(end >= read_begin());
    assert(end <= write_begin());

    retrieve(end - read_begin());
}

void Buffer::retrieve_int64()
{
    retrieve(sizeof(std::int64_t));
}

void Buffer::retrieve_int32()
{
    retrieve(sizeof(std::int32_t));
}

void Buffer::retrieve_int16()
{
    retrieve(sizeof(std::int16_t));
}

void Buffer::retrieve_int8()
{
    retrieve(sizeof(std::int8_t));
}

std::string Buffer::retrieve_all_as_string()
{
    return retrieve_as_string(readable_bytes());
}

std::string Buffer::retrieve_as_string(std::size_t len)
{
    assert(len <= readable_bytes());
    std::string result{ read_begin(), len };
    retrieve(len);
    return result;
}

StringView Buffer::to_string_view() const
{
    return StringView{ read_begin(), readable_bytes() };
}

void Buffer::append(const StringView& sv)
{
    append(sv.data(), sv.size());
}

void Buffer::append(const char* str, std::size_t len)
{
    ensure_writable_bytes(len);
    std::copy(str, str + len, write_begin());
    add_writer_index(len);
}

void Buffer::append(const void* data, std::size_t len)
{
    append(static_cast<const char*>(data), len);
}

void Buffer::append_uint64(std::uint64_t x)
{
    x = host_to_net64(x);
    append(&x, sizeof(x));
}

void Buffer::append_uint32(std::uint32_t x)
{
    x = host_to_net32(x);
    append(&x, sizeof(x));
}

void Buffer::append_uint16(std::uint16_t x)
{
    x = host_to_net16(x);
    append(&x, sizeof(x));
}

void Buffer::append_uint8(std::uint8_t x)
{
    append(&x, sizeof(x));
}

ssize_t Buffer::read_fd(int fd, int& savedError)
{
    char extrabuf[65536];
    struct iovec vec[2];
    const std::size_t writable = writable_bytes();

    vec[0].iov_base = write_begin();
    vec[0].iov_len  = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len  = sizeof(extrabuf);

    // when there is enough space in the buffer, don't read into extrabuf
    const int iovcnt = writable < sizeof(extrabuf) ? 2 : 1;
    const ssize_t n = ::readv(fd, vec, iovcnt);
    if (n < 0)
    {
        savedError = errno;
    }
    else if (static_cast<std::size_t>(n) <= writable)
    {
        mWriterIndex += static_cast<std::size_t>(n);
    }
    else
    {
        mWriterIndex += mBuffer.size();
        append(extrabuf, n - writable);
    }

    return n;
}

void Buffer::ensure_writable_bytes(std::size_t len)
{
    if (writable_bytes() < len)
    {
        // make space
        if (writable_bytes() + prepend_bytes() < len + kPrependSize)
        {
            // mBuffer can't hold the data
            mBuffer.resize(len + mWriterIndex);
        }
        else
        {
            // mBuffer can hold the data
            // only move forward 
            assert(kPrependSize < mReaderIndex);
            std::size_t readable = readable_bytes();
            std::copy(begin() + mReaderIndex,
                      begin() + mWriterIndex,
                      begin() + kPrependSize);
            // adjust the index
            mReaderIndex = kPrependSize;
            mWriterIndex = mReaderIndex + readable;
            assert(readable == readable_bytes());
        }
    }
    assert(writable_bytes() >= len);
}

void Buffer::add_writer_index(std::size_t len)
{
    assert(len <= writable_bytes());
    mWriterIndex += len;
}

void Buffer::sub_writer_index(std::size_t len)
{
    assert(len <= readable_bytes());
    mWriterIndex -= len;
}

} // namespace Net

} // namespace Asuka