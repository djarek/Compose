//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_IMPL_UNSTABLE_TRANSFORM_HPP
#define COMPOSE_IMPL_UNSTABLE_TRANSFORM_HPP

#include <compose/transformed_operation.hpp>
#include <compose/unstable_transform.hpp>

namespace compose
{

namespace detail
{

template<typename OperationBody,
         typename Executor,
         typename CompletionToken,
         typename Signature,
         typename... Args>
auto
unstable_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  Args&&... args)
{
    static_assert(boost::asio::is_executor<Executor>::value,
                  "Executor must be an Executor");

    using upcall_t =
      upcall_op<BOOST_ASIO_HANDLER_TYPE(CompletionToken, Signature), Executor>;
    using op_t = composed_op<OperationBody, upcall_t, false>;
    return transformed_operation<op_t>{
      upcall_t{std::move(init.completion_handler), ex},
      std::forward<Args>(args)...};
}

} // namespace detail

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
  Args&&... args)
{
    return detail::unstable_transform<OperationBody>(
      ex, init, std::forward<Args>(args)...);
}

template<typename OperationBody,
         typename Executor,
         typename CompletionToken,
         typename Signature>
auto
unstable_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  OperationBody&& cb)
{
    return detail::unstable_transform<typename std::decay<OperationBody>::type>(
      ex, init, std::forward<OperationBody>(cb));
}

} // namespace compose

#endif // COMPOSE_IMPL_UNSTABLE_TRANSFORM_HPP
