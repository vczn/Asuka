#pragma once
#ifndef ASUKA_WEAK_CALLBACK_HPP
#define ASUKA_WEAK_CALLBACK_HPP

#include <memory>

namespace Asuka
{

template <typename T, typename... Args>
class WeakCallback
{
public:
    WeakCallback(const std::weak_ptr<T>& obj,
        std::function<void(T*, Args...)> function)
        : mObject(obj),
          mFunction(std::move(function))
    {
    }

    void operator()(Args&&... args) const
    {
        std::shared_ptr<T> sp(mObject.lock());
        if (sp)
        {
            mFunction(sp.get(), std::forward<Args>(args)...);
        }
    }
private:
    std::weak_ptr<T> mObject;
    std::function<void(T*, Args...)> mFunction;
};

template <typename T, typename... Args>
WeakCallback<T, Args...> make_weak_callback(const std::shared_ptr<T>& obj,
    void (T::*function)(Args...))
{
    return WeakCallback<T, Args...>(obj, function);
}

template <typename T, typename... Args>
WeakCallback<T, Args...> make_weak_callback(const std::shared_ptr<T>& obj,
    void(T::*function)(Args...) const)
{
    return WeakCallback<T, Args...>(obj, function);
}

} // namespace Asuka

#endif // ASUKA_WEAK_CALLBACK_HPP
