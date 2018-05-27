// Part of Asuka net utility, internal header
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_BUFFER_HPP
#define ASUKA_BUFFER_HPP

#include <vector>

#include "../util/string_view.hpp"

namespace Asuka
{

namespace Net
{ 

// reference muduo
// https://github.com/chenshuo/muduo/blob/master/muduo/net/Buffer.h

// 
// +--------------------+----------------+----------------+
// |                    |                |                |
// |  prependable bytes | readable bytes | writable bytes |
// |                    |                |                |
// +--------------------+----------------+----------------+
// |                    |                |                |
// 0                    reader_index     writer_index     size
// begin                                                  end


class Buffer
{
public:
    explicit Buffer(std::size_t initSize = kInitialSize);
    Buffer(const Buffer& rhs);
    Buffer(Buffer&& rhs) noexcept;

    void swap(Buffer& rhs) noexcept;

    Buffer& operator=(const Buffer& rhs);
    Buffer& operator=(Buffer&& rhs) noexcept;

    std::size_t readable_bytes() const;
    std::size_t writable_bytes() const;
    std::size_t prepend_bytes() const;

    // return the pointer to where start reading
    const char* peek() const;

    // find crlf, "\r\n"
    const char* find_crlf() const;
    const char* find_crlf(const char* start) const;

    // find eol, "\n"
    const char* find_eol() const;
    const char* find_eol(const char* start) const;

    // the pointer to where start reading
    char* read_begin();
    const char* read_begin() const;

    // the pointer to where start writing
    char* write_begin();
    const char* write_begin() const;

    // mBuffer.data() + 0
    char* begin();
    const char* begin() const;

    // mBuffer.data() + mBuffer.size()
    char* end();
    const char* end() const;

    std::size_t size() const;
    std::size_t capacity() const;

    void reserve(std::size_t sizeBuf);
    void shrink_to_fit();

    // advances the reading index of the buffer up len 
    // if len == readable_bytes, reset
    void retrieve(std::size_t len);
    void retrieve_all();
    void retrieve_util(const char* end);    // [read_begin, end)

    void retrieve_int64();
    void retrieve_int32();
    void retrieve_int16();
    void retrieve_int8();

    std::string retrieve_all_as_string();
    std::string retrieve_as_string(std::size_t len);

    // convert read buffer to string view
    StringView to_string_view() const;

    // append data at read buffer
    void append(const StringView& sv);
    void append(const char* str, std::size_t len);
    void append(const void* data, std::size_t len);

    // append x at read buffer with net endian
    void append_uint64(std::uint64_t x);
    void append_uint32(std::uint32_t x);
    void append_uint16(std::uint16_t x);
    void append_uint8(std::uint8_t x);

    // read data directly into buffer
    // on success, return the number of bytes read is returned
    // on error, return -1 and `savedError` is set appropriately
    ssize_t read_fd(int fd, int& savedError);
private:
    void ensure_writable_bytes(std::size_t len);
    void add_writer_index(std::size_t len);
    void sub_writer_index(std::size_t len);

private:
    std::vector<char> mBuffer;
    std::size_t mReaderIndex;
    std::size_t mWriterIndex;

    static const char kCRLF[];
    static const std::size_t kInitialSize;
    static const std::size_t kPrependSize;
};

} // namespace Net

} // namespace Asuka

#endif // ASUKA_BUFFER_HPP