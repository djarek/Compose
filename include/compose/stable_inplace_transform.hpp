//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_STABLE_INPLACE_TRANSFORM_HPP
#define COMPOSE_STABLE_INPLACE_TRANSFORM_HPP

#include <compose/stable_transform.hpp>

namespace compose
{

template<typename T>
struct converter
{
    T make_result;

    static T& declval();
    using result_type = decltype(converter::declval()());

    template<typename U>
    operator U()
    {
        return make_result();
    }
};

template<typename T>
converter(T t)->converter<T>;

template<typename Executor,
         typename CompletionToken,
         typename Signature,
         typename F>
auto
stable_inplace_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  converter<F> conv)
{
    return detail::stable_transform<typename converter<F>::result_type>(
      ex, init, conv);
}

#define COMPOSE_INPLACE_FWD(...)                                               \
    converter                                                                  \
    {                                                                          \
        [&]() { return (__VA_ARGS__); }                                        \
    }

} // namespace compose

#endif // COMPOSE_STABLE_INPLACE_TRANSFORM_HPP
