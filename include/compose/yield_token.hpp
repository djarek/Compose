//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_YIELD_TOKEN_HPP
#define COMPOSE_YIELD_TOKEN_HPP

#include <compose/upcall_guard.hpp>

#include <boost/asio/async_result.hpp>
#include <type_traits>

namespace compose
{

/**
 * A proxy type that represents the currently running composed operation.
 *
 * Some member functions are potentially destructive,
 * i.e. references to the associated OperationBody or its data members may
 * become invalid and the token is placed in a valid, but unspecified state.
 */
template<typename ComposedOp>
class yield_token
{
public:
    /**
     * Construct a yield_token
     */
    yield_token(ComposedOp& op, bool is_continuation);

    /**
     * Releases ownership of the composed operation, which may be used as a
     * CompletionHandler in an asynchronous operation.
     *
     * @remark The yield_token is put in a moved-from state.
     */
    ComposedOp release_operation() const;

    /**
     * Performs a post-upcall to the CompletionHandler associated with the
     * referenced composed operation. The associated CompletionHandler will be
     * posted to the associated executor and then invoked with the bound
     * arguments.
     *
     * @param args Arguments that will be bound to the associated
     * CompletionHandler, as-if by using boost::beast::bind_handler()
     *
     * @remark May invalidate references to the composed operation body or its
     * data members.
     */
    template<typename... Args>
    upcall_guard post_upcall(Args&&... args);

    /**
     * Performs a direct-upcall to the CompletionHandler associated with the
     * referenced composed operation. The associated CompletionHandler will be
     * invoked in place. The handler may be moved onto the stack before the
     * upcall if required. Behavior of this function is undefined if the
     * currently running operation is not a continuation or the associated
     * executor's invariant has not been upheld.
     *
     * @param args Arguments that will be passed to the invocation of the
     * associated CompletionHandler.
     *
     * @remark May invalidate references to the composed operation body or its
     * data members.
     */
    template<typename... Args>
    upcall_guard direct_upcall(Args&&... args);

    /**
     * Performs an upcall to the CompletionHandler associated with the
     * referenced composed operation. The upcall type is selected based on
     * whether the currently running composed operation is a continuation.
     *
     * @param args Arguments that will be bound to the associated
     * CompletionHandler, as-if by using boost::beast::bind_handler()
     *
     * @remark May invalidate references to the composed operation body or its
     * data members.
     */
    template<typename... Args>
    upcall_guard upcall(Args&&... args);

    /**
     * Indicates whether the currently running composed operation is a
     * continuation.
     */
    bool is_continuation() const noexcept
    {
        return is_continuation_;
    }

private:
    ComposedOp& op_;
    bool is_continuation_;
};

} // namespace compose

namespace boost
{
namespace asio
{

template<typename ComposedOp, typename Signature>
class async_result<compose::yield_token<ComposedOp>, Signature>
{
public:
    using return_type = compose::upcall_guard;
    using completion_handler_type = ComposedOp;

    explicit async_result(completion_handler_type&)
    {
    }

    return_type get()
    {
        return {};
    }
};

} // namespace asio
} // namespace boost

#include <compose/impl/yield_token.hpp>

#endif // COMPOSE_YIELD_TOKEN_HPP
