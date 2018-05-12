// Part of Asuka utility, internal header
// Copyleft 2018, vczn

// types helper header, implements some c++17 type traits
#pragma once
#ifndef ASUKA_TYPE_TRAITS_HPP
#define ASUKA_TYPE_TRAITS_HPP

#include <type_traits>

namespace Asuka
{

// logical AND
template <typename...>
struct conjunction : std::true_type {};

template <typename B>
struct conjunction<B> : B {};

template <typename B1, typename... Bn>
struct conjunction<B1, Bn...>
    : std::conditional_t<static_cast<bool>(B1::value), 
        conjunction<Bn...>, B1> { };

template <typename... B>
constexpr bool conjunction_v = conjunction<B...>::value;

// logical OR
template <typename...>
struct disjunction : std::false_type {};

template <typename B>
struct disjunction<B> : B {};

template <typename B1, typename... Bn>
struct disjunction<B1, Bn...>
    : std::conditional_t<static_cast<bool>(B1::value),
    B1, disjunction<Bn...>> { };

template <typename... B>
constexpr bool disjunction_v = disjunction<B...>::value;


// logical NOT
template <typename Traits>
struct negation : std::__bool_constant<
    !static_cast<bool>(Traits::value)>
{
};

template <typename Traits>
constexpr bool negation_v = negation<Traits>::value;


template <typename T, typename... Args>
constexpr bool is_constructible_v = std::is_constructible<T, Args...>::value;

template <typename T>
constexpr bool is_copy_constructible_v = std::is_copy_constructible<T>::value;

template <typename T, typename U>
constexpr bool is_same_v = std::is_same<T, U>::value;

}

#endif // ASUKA_TYPE_TRAITS_HPP