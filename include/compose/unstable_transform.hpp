//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_UNSTABLE_TRANSFORM_HPP
#define COMPOSE_UNSTABLE_TRANSFORM_HPP

#include <compose/detail/composed_operation.hpp>
#include <compose/yield_token.hpp>

namespace compose
{

/**
 * Performs a transformation of an OperationBody object into a
 * ComposedOperation.
 *
 * @remark Passing a non-const reference to a subobject of OperationBody to an
 * async-initiation function may result in the reference becoming invalid and
 * invoking Undefined Behavior. Use a stable version of this function if the
 * OperationBody must retain state for the duration of the operation.
 *
 * @tparam OperationBody the type that will transformed into a
 * ComposedOperation. The OperationBody is required to satisfy the constraints
 * of MoveConstructible and no guarantees are provided for the address stability
 * of that object.
 *
 * @param ex The I/O executor to be used by the ComposedOperation (associated
 * with the used I/O object).
 *
 * @param init Reference to an asynchronous operation initiation helper. Used to
 * deduce the CompletionHandler and Signature types.
 *
 * @param args Arguments forwarded to the constructor of OperationBody.
 *
 * @returns transformed_operation<DEDUCED>
 */
template<typename OperationBody,
         typename Executor,
         typename CompletionToken,
         typename Signature,
         typename... Args>
auto
unstable_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  std::piecewise_construct_t,
  Args&&... args);

/**
 * Performs a transformation of an OperationBody object into a
 * ComposedOperation.
 *
 * @remark Passing a non-const reference to a subobject of OperationBody to an
 * async-initiation function may result in the reference becoming invalid and
 * invoking Undefined Behavior. Use a stable version of this function if the
 * OperationBody must retain state for the duration of the operation.
 *
 * @param ex The I/O executor to be used by the ComposedOperation (associated
 * with the used I/O object).
 *
 * @param init Reference to an asynchronous operation initiation helper. Used to
 * deduce the CompletionHandler and Signature types.
 *
 * @param ob The operation that will transformed into a
 * ComposedOperation. The OperationBody is required to satisfy the constraints
 * of MoveConstructible and no guarantees are provided for the address stability
 * of that object
 *
 * @returns transformed_operation<DEDUCED>
 */
template<typename OperationBody,
         typename Executor,
         typename CompletionToken,
         typename Signature>
auto
unstable_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  OperationBody&& ob);

template<typename OperationBody, typename Executor, typename CompletionHandler>
using unstable_yield_token_t = yield_token<
  detail::composed_op<OperationBody, CompletionHandler, Executor, false>>;

} // namespace compose

#include <compose/impl/unstable_transform.hpp>

#endif // COMPOSE_UNSTABLE_TRANSFORM_HPP
