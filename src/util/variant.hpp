
// ------------------------------------------
// temporarily use the `boost` library
// ------------------------------------------

//#include <limits>
//#include <memory>
//#include <new>
//#include <tuple>
//#include <type_traits>
//#include <typeindex>
//
//#include "type_traits.hpp"
//
//#pragma once
//#ifndef ASUKA_VARIANT_HPP
//#define ASUKA_VARIANT_HPP
//
//namespace Asuka
//{ 
//
//template <std::size_t kFirst, std::size_t kSecond>
//struct BinMaxSize : std::integral_constant<std::size_t,
//    (kFirst < kSecond) ? kSecond : kFirst>
//{
//};
//
//template <std::size_t kFirst, std::size_t... kRest>
//struct MaxSize : std::integral_constant<std::size_t,
//    BinMaxSize<kFirst, MaxSize<kRest...>::value>::value>
//{
//};
//
//template <std::size_t kFirst>
//struct MaxSize<kFirst> : std::integral_constant<std::size_t, kFirst>
//{
//};
//
//template <typename... Ts>
//struct VariantCheckType
//{
//};
//
//template <typename First>
//struct VariantCheckType<First>
//{
//    static_assert(!std::is_reference<First>::value && !std::is_array<First>::value
//        && !std::is_void<First>::value, "not reference or raw array or void");
//};
//
//template <typename First, typename... Rest>
//struct VariantCheckType<First, Rest...> : private VariantCheckType<Rest...>
//{
//    static_assert(!std::is_reference<First>::value && !std::is_array<First>::value
//        && !std::is_void<First>::value, "not reference or raw array or void");
//};
//
//
//template <typename... Ts>
//struct VariantStorage : private VariantCheckType<Ts...>
//{
//    alignas(MaxSize<alignof(Ts)...>::value) char s[MaxSize<sizeof(Ts)...>::value];
//};
//
//using Index_t = unsigned int;
//
//
//template <Index_t index, typename... Ts>
//struct IndexType;
//
//
//template <Index_t index, typename First, typename... Rest>
//struct IndexType<index, First, Rest...>
//    : IndexType<index - 1, Rest...>
//{
//};
//
//template <typename First, typename... Rest>
//struct IndexType<0, First, Rest...>
//{
//    using type = First;
//};
//
//
//template <typename... Ts>
//struct VariantHelper;
//
//
//template <typename Head, typename... Rest>
//struct VariantHelper<Head, Rest...>
//{
//    static void vdestroy(Index_t index, void* pStorage)
//    {
//        if (index == 0)
//        {
//            reinterpret_cast<Head*>(pStorage)->~Head();
//        }
//        else
//        {
//            VariantHelper<Rest...>::vdestroy(index - 1, pStorage);
//        }
//    }
//
//    static void vcopy(Index_t index, const void* pRhs, void* pStorage)
//    {
//        if (index == 0)
//        {
//            ::new (pStorage) Head(*reinterpret_cast<const Head*>(pRhs));
//        }
//        else
//        {
//            VariantHelper<Rest...>::vcopy(index - 1, pRhs, pStorage);
//        }
//    }
//
//    static void vmove(Index_t index, void* pRhs, void* pStorage)
//    {
//        if (index == 0)
//        {
//            ::new (pStorage) Head(std::move(*reinterpret_cast<Head*>(pRhs)));
//        }
//        else
//        {
//            VariantHelper<Rest...>::vmove(index - 1, pRhs, pStorage);
//        }
//    }
//};
//
//template <>
//struct VariantHelper<>
//{
//    static void vdestroy(Index_t, void*) { }
//    static void vcopy(Index_t, const void*, void*) { }
//    static void vmove(Index_t, void*, void*) { }
//};
//
//template <typename... Ts>
//class Variant
//{
//    static_assert(sizeof...(Ts) > 0, "Variant<> is invalid");
//    static_assert(sizeof...(Ts) < std::numeric_limits<Index_t>::max(), "number of Ts is too many");
//public:
//    constexpr std::size_t variant_npos = -1;
//    
//    
//private:
//    static constexpr const Index_t kInvalidIndex = static_cast<Index_t>(-1);
//
//    using HelperType = VariantHelper<Ts...>;
//    using TupleHelper = std::tuple<Ts...>;
//    using FirstType = typename std::tuple_element<0, TupleHelper>::type;
//public:
//    constexpr Variant() noexcept 
//        : mWhickIndex(static_cast<Index_t>(-1))
//    {
//        static_assert(std::is_default_constructible<FirstType>::value,
//            "First type must be default constructible");
//        ::new (&mVStorage) FirstType();
//    }
//
//    Variant(const Variant& rhs)
//        : mIndex(rhs.mIndex)
//    {
//        HelperType::vcopy(rhs.mIndex, &rhs.mVStorage, &mVStorage);
//    }
//
//    Variant(Variant&& rhs)
//    {
//        HelperType::vmove(rhs.mIndex, &rhs.mVStorage, &mVStorage);
//    }
//
//    //template <typename t>
//    //variant(t&& val)
//    //{
//    //    
//    //}
//
//    ~Variant()
//    {
//        HelperType::vdestroy()
//    }
//
//    constexpr bool valueless_by_exception() const noexcept
//    {
//        return mIndex == static_cast<Index_t>(-1);
//    }
//private:
//
//private:
//    VariantStorage<Ts...> mVStorage;
//    Index_t mIndex;
//};
//
//
//} // namespace Asuka
//
//#endif // ASUKA_VARIANT_HPP
