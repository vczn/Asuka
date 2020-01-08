#pragma once
#ifndef ASUKA_BLOCK_QUEUE_HPP
#define ASUKA_BLOCK_QUEUE_HPP

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <queue>

#include "noncopyable.hpp"

namespace Asuka
{

// Thread queue
template <typename T>
class BlockQueue : Noncopyable
{
public:
    BlockQueue() 
        : mMutex(),
          mCond(),
          mData()
    {
    }

    void push(T value)
    {
        std::lock_guard<std::mutex> lock{ mMutex };
        mData.push(std::move(value));
        mCond.notify_one();
    }

    template <typename... Args>
    void emplace(Args&&... args)
    {
        std::lock_guard<std::mutex> lock{ mMutex };
        mData.emplace(std::forward<Args>(args)...);
        mCond.notify_one();
    }

    void pop(T& value)
    {
        std::lock_guard<std::mutex> lock{ mMutex };
        mCond.wait(mMutex, [this]() { return !mData.empty(); });
        assert(!mData.empty());

        value = std::move(mData.front());
        mData.pop();
    }

    std::size_t size() const
    {
        std::lock_guard<std::mutex> lock{ mMutex };
        return mData.size();
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock{ mMutex };
        return mData.empty();
    }
private:
    mutable std::mutex mMutex;
    std::condition_variable mCond;
    std::queue<T> mData;
};

} // namespace Asuka

#endif // ASUKA_BLOCK_QUEUE_HPP