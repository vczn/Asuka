// Part of Asuka utility 
// Copyleft 2018, vczn

#pragma once
#ifndef ASUKA_CXX_VERSION_HPP
#define ASUKA_CXX_VERSION_HPP

static_assert(__cplusplus >= 201103L, "at least c++11");

#if __cplusplus >= 201103L
#  define ASUKACXX11 1
#endif // __cplusplus >= 201103L

#if __cplusplus >= 201402L
#  define ASUKACXX14 1
#endif // __cplusplus >= 201402L

// CXXVER11 must be defined
#ifdef ASUKACXX11
#  define CONSTEXPR11   constexpr
#  define NOEXCEPT      noexcept
#  define OVERRIDE      override
#  define FINAL         final
#endif // CXXVER11

#ifdef ASUKACXX14
#  define CONSTEXPR14   constexpr
#else 
#  define CONSTEXPR14   inline
#endif // CXXVER14

#endif // ASUKA_CXX_VERSION_HPP
