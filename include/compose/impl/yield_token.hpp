//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_IMPL_YIELD_TOKEN_HPP
#define COMPOSE_IMPL_YIELD_TOKEN_HPP

#include <compose/yield_token.hpp>
#include <compose/detail/running_in_this_thread.hpp>

#include <boost/assert.hpp>

namespace compose
{

template<typename ComposedOp>
template<typename... Args>
upcall_guard
yield_token<ComposedOp>::post_upcall(Args&&... args) &&
{
    return op_.post_upcall(std::forward<Args>(args)...);
}

template<typename ComposedOp>
template<typename... Args>
upcall_guard
yield_token<ComposedOp>::direct_upcall(Args&&... args) &&
{
    BOOST_ASSERT(is_continuation_ && "Direct upcall can only be used in a "
                                     "continuation. Use post_upcall instead.");
    BOOST_ASSERT(detail::running_in_this_thread(op_.get_executor(), nullptr) &&
                 "Direct upcall must not be performed outside of the "
                 "CompletionHandler's Executor context.");
    return op_.direct_upcall(std::forward<Args>(args)...);
}

template<typename ComposedOp>
template<typename... Args>
upcall_guard
yield_token<ComposedOp>::upcall(Args&&... args) &&
{
    if (is_continuation_)
        return std::move(*this).direct_upcall(std::forward<Args>(args)...);
    else
        return std::move(*this).post_upcall(std::forward<Args>(args)...);
}

} // namespace compose

#endif // COMPOSE_IMPL_YIELD_TOKEN_HPP
