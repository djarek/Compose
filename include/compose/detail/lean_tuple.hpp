//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_LEAN_TUPLE
#define COMPOSE_DETAIL_LEAN_TUPLE

#include <boost/mp11/integer_sequence.hpp>

#if __cplusplus > 201703L
#define COMPOSE_USE_NO_UNIQUE_ADDRESS
#else
#include <type_traits>
#endif

namespace compose
{
namespace detail
{

#ifndef COMPOSE_USE_NO_UNIQUE_ADDRESS

template<typename T>
struct use_ebo
{
    static auto constexpr value =
      !std::is_final<T>::value && std::is_empty<T>::value;
};

template<std::size_t I, typename T, bool = use_ebo<T>::value>
struct tuple_element;

template<std::size_t I, typename T>
struct tuple_element<I, T, true> : T
{
#if __cplusplus < 201703L
    template<typename U>
    explicit tuple_element(U&& u)
      : T(u)
    {
    }
#endif

    static T& get_helper(tuple_element& te)
    {
        return te;
    }

    static T const& get_helper(tuple_element const& te)
    {
        return te;
    }
};

template<std::size_t I, typename T>
struct tuple_element<I, T, false>
{
    static T& get_helper(tuple_element& te)
    {
        return te.t_;
    }

    static T const& get_helper(tuple_element const& te)
    {
        return te.t_;
    }

    T t_;
};

#else // COMPOSE_USE_NO_UNIQUE_ADDRESS

template<std::size_t I, typename T>
struct tuple_element
{
    static T& get_helper(tuple_element& te)
    {
        return te.t_;
    }

    static T const& get_helper(tuple_element const& te)
    {
        return te.t_;
    }

    [[no_unique_address]] T t_;
};

#endif // COMPOSE_USE_NO_UNIQUE_ADDRESS

template<typename... Ts>
struct tuple_impl;

template<typename... Ts, std::size_t... Is>
struct tuple_impl<boost::mp11::index_sequence<Is...>, Ts...>
  : tuple_element<Is, Ts>...
{
#if __cplusplus < 201703L
    tuple_impl() = default;

    template<typename... Us>
    explicit tuple_impl(Us&&... us)
      : tuple_element<Is, Ts>{static_cast<Us&&>(us)}...
    {
    }
#endif
};

template<typename... Ts>
struct lean_tuple : tuple_impl<boost::mp11::index_sequence_for<Ts...>, Ts...>
{
#if __cplusplus < 201703L
    template<typename... Us>
    explicit lean_tuple(Us&&... us)
      : tuple_impl<boost::mp11::index_sequence_for<Ts...>, Ts...>{
          static_cast<Us&&>(us)...}
    {
    }
#endif
};

template<std::size_t I, typename T>
T&
get(tuple_element<I, T>& te)
{
    return tuple_element<I, T>::get_helper(te);
}

template<std::size_t I, typename T>
T const&
get(tuple_element<I, T> const& te)
{
    return tuple_element<I, T>::get_helper(te);
}

template<std::size_t I, typename T>
T&&
get(tuple_element<I, T>&& te)
{
    return static_cast<T&&>(tuple_element<I, T>::get_helper(te));
}

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_LEAN_TUPLE
