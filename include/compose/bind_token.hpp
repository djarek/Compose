//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_BIND_TOKEN_HPP
#define COMPOSE_BIND_TOKEN_HPP

#include <compose/detail/bind_handler_front.hpp>

namespace compose
{
template<typename ComposedOp>
class yield_token;

/**
 * Binds arguments with a yield_token creating a new CompletionHandler for the
 * purpose of tag dispatch/invoking a particular overload of operator().
 *
 * @param token Source object which will be used to construct the
 * CompletionHandler. Must be in a valid state. May place the currently running
 * operation body in a moved-from state.
 *
 * @param args Arguments to be bound at the front of the handler argument list
 */
template<typename ComposedOp, typename... Args>
auto
bind_token(yield_token<ComposedOp>&& token, Args&&... args)
  -> detail::bound_front_op<detail::remove_cref_t<ComposedOp>,
                            detail::remove_cref_t<Args>...>
{
    return {compose::detail::bind_handler_front(
      std::move(token).release_operation(), std::forward<Args>(args)...)};
}

} // namespace compose

#endif // COMPOSE_COMPOSED_OPERATION_HPP
