#pragma once
#ifndef ASUKA_ANY_HPP
#define ASUKA_ANY_HPP

#include <algorithm>
#include <typeinfo>

#include "type_traits.hpp"

namespace Asuka
{

// reference the boost

class BadAnyCast : public std::bad_cast
{
public:
    const char* what() const noexcept override
    {
        return "bad any_cast";
    }
};

// Any can hold any type
class Any
{
public:
    constexpr Any() noexcept 
        : mContent(nullptr)
    {
    }

    template <typename T, std::enable_if_t<
        conjunction_v<
            negation<std::is_same<std::decay_t<T>, Any>>,
            std::is_copy_constructible<std::decay_t<T>>>, 
        int> = 0>
    Any(T&& value)
        : mContent(new Holder<std::decay_t<T>>(std::forward<T>(value)))
    {
    }

    Any(const Any& rhs)
        : mContent(rhs.mContent ? rhs.mContent->clone() : nullptr)
    {
    }

    Any(Any&& rhs) noexcept
        : mContent(rhs.mContent)
    {
        rhs.mContent = nullptr;
    }

    ~Any()
    {
        if (mContent)
        {
            delete mContent;
        }
    }

    Any& operator=(const Any& rhs)
    {
        if (this != &rhs)
        {
            Any(rhs).swap(*this);
        }
        
        return *this;
    }

    Any& operator=(Any&& rhs) noexcept
    {
        if (this != &rhs)
        {
            rhs.swap(*this);
            Any().swap(rhs);
        }
        
        return *this;
    }

    template <typename T, std::enable_if_t<
        conjunction_v<
            negation<std::is_same<std::decay_t<T>, Any>>,
            std::is_copy_constructible<std::decay_t<T>>>, 
        int> = 0>
    Any& operator=(T&& rhs)
    {
        Any(std::forward<T>(rhs)).swap(*this);
        return *this;
    }

    void swap(Any& rhs) noexcept
    {
        std::swap(mContent, rhs.mContent);
    }

    bool has_value() const noexcept
    {
        return mContent;
    }

    void reset() noexcept
    {
        Any().swap(*this);
    }

    const std::type_info& type() const noexcept
    {
        if (mContent)
        {
            return mContent->get_type();
        }
      
        return typeid(void);
    }
private:
    class PlaceHolder
    {
    public:
        virtual ~PlaceHolder()
        {
        }

        virtual const std::type_info& get_type() const noexcept = 0;
        virtual PlaceHolder* clone() const = 0;
    }; // PlaceHolder

    template <typename T>
    class Holder : public PlaceHolder
    {
    public:
        // copy
        Holder(const T& value)
            : held(value)
        {
        }

        // move
        Holder(T&& value) 
            : held(std::move(value))
        {
        }

        const std::type_info& get_type() const noexcept override
        {
            return typeid(T);
        }

        PlaceHolder* clone() const override
        {
            return new Holder{ held };
        }

        T held;
    }; // Holder

private:
    PlaceHolder* mContent;

    template <typename T>
    friend T* any_cast(Any*) noexcept;
};

inline void swap(Any& lhs, Any& rhs)
{
    lhs.swap(rhs);
}

template <typename T>
inline T* any_cast(Any* operand) noexcept
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    return operand && operand->type() == typeid(T)
        ? std::addressof(static_cast<Any::Holder<U>*>(operand->mContent)->held)
        : nullptr;
}

template <typename T>
inline const T* any_cast(const Any* operand) noexcept
{
    return static_cast<const T*>(
        any_cast<T>(const_cast<Any*>(operand)));
}

template <typename T>
inline T any_cast(const Any& operand)
{
    using U = const std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(is_constructible_v<T, const U&>, 
        "any_cast<T>(const Any&) requires T to be constructible"
        "from const remove_cv_t<remove_reference_t<T>>&");
    
    U* result = any_cast<U>(std::addressof(operand));
    if (!result)
        throw BadAnyCast{};

    return static_cast<T>(*result);
}

template <typename T>
inline T any_cast(Any& operand)
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(is_constructible_v<T, U&>,
        "any_cast<T>(Any&) requires T to be constructible"
        "from remove_cv_t<remove_reference_t<T>>&");

    U* result = any_cast<U>(std::addressof(operand));
    if (!result)
        throw BadAnyCast{};

    return static_cast<T>(*result);
}

template <typename T>
T any_cast(Any&& operand)
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(is_constructible_v<T, U>,
        "any_cast<T>(Any&&) requires T to be constructible"
        "from remove_cv_t<remove_reference_t<T>>");

    U* result = any_cast<U>(std::addressof(operand));
    if (!result)
        throw BadAnyCast{};

    return static_cast<T>(std::move(*result));
}

} // namespace Asuka

#endif // ASUKA_ANY_HPP
