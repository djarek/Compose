// < Argsstd::forward<Args>(args)
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_IMPL_YIELD_TOKEN_HPP
#define COMPOSE_IMPL_YIELD_TOKEN_HPP

#include <compose/detail/running_in_this_thread.hpp>
#include <compose/yield_token.hpp>

#include <boost/asio/associated_executor.hpp>

#include <cassert>
#include <utility>

namespace compose
{

template<typename ComposedOp>
yield_token<ComposedOp>::yield_token(ComposedOp& op, bool is_continuation)
  : op_{op}
  , is_continuation_{is_continuation}
{
}

template<typename ComposedOp>
ComposedOp
yield_token<ComposedOp>::release_operation() const
{
    return std::move(op_);
}

template<typename ComposedOp>
template<typename... Args>
upcall_guard
yield_token<ComposedOp>::post_upcall(Args&&... args)
{
    op_.post_upcall(std::forward<Args>(args)...);
    return {};
}

template<typename ComposedOp>
template<typename... Args>
upcall_guard
yield_token<ComposedOp>::direct_upcall(Args&&... args)
{
    assert(is_continuation_ && "Direct upcall can only be used in a "
                               "continuation. Use post_upcall instead.");
    assert(detail::running_in_this_thread(
             boost::asio::get_associated_executor(op_), nullptr) &&
           "Direct upcall must not be performed outside of the "
           "CompletionHandler's Executor context.");
    op_.direct_upcall(std::forward<Args>(args)...);
    return {};
}

template<typename ComposedOp>
template<typename... Args>
upcall_guard
yield_token<ComposedOp>::upcall(Args&&... args)
{
    if (is_continuation_)
        return direct_upcall(std::forward<Args>(args)...);
    else
        return post_upcall(std::forward<Args>(args)...);
}

} // namespace compose

#endif // COMPOSE_IMPL_YIELD_TOKEN_HPP
