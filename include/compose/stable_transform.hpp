//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_STABLE_TRANSFORM_HPP
#define COMPOSE_STABLE_TRANSFORM_HPP

#include <compose/detail/composed_operation.hpp>
#include <compose/yield_token.hpp>

namespace compose
{

/**
 * Performs a transformation of an OperationBody object into a
 * ComposedOperation. This transformation guarantees that the OperationBody
 * object will remain at the same address (no calls to it's Move or Copy
 * constructors will be made) for the duration of the ComposedOperation (until
 * upcall is performed or the ComposedOperation is discarded).
 *
 * Performs one additional memory allocation, using the Allocator associated
 * with the deduced CompletionHandler. The memory persists until upcall or the
 * operation is discarded.
 *
 * @tparam OperationBody the type that will transformed into a
 * ComposedOperation.
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
stable_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  std::piecewise_construct_t,
  Args&&... args);

/**
 * Performs a transformation of an OperationBody object into a
 * ComposedOperation. This transformation guarantees that the OperationBody
 * object will remain at the same address for the duration of the
 * ComposedOperation (until upcall is performed or the ComposedOperation is
 * discarded). The OperationBody's Move or Copy constructor is guaranteed to be
 * invoked at most once, during construction of the ComposedOperation.
 *
 * Performs one additional memory allocation, using the Allocator associated
 * with the deduced CompletionHandler. The memory persists until upcall or the
 * operation is discarded.
 *
 * @param ex The I/O executor to be used by the ComposedOperation (associated
 * with the used I/O object).
 *
 * @tparam OperationBody the type that will transformed into a
 * ComposedOperation.
 * @param init Reference to an asynchronous operation initiation helper. Used to
 * deduce the CompletionHandler and Signature types.
 *
 * @param args Arguments forwarded to the constructor of OperationBody.
 *
 * @returns transformed_operation<DEDUCED>
 */
template<typename Executor,
         typename CompletionToken,
         typename Signature,
         typename OperationBody>
auto
stable_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  OperationBody&& cb);

template<typename OperationBody, typename Executor, typename CompletionHandler>
using stable_yield_token_t = yield_token<
  detail::composed_op<OperationBody,
                      detail::upcall_op<CompletionHandler, Executor>,
                      true>&>;

} // namespace compose

#include <compose/impl/stable_transform.hpp>

#endif // COMPOSE_STABLE_TRANSFORM_HPP
