//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_IMPL_STABLE_TRANSFORM_HPP
#define COMPOSE_IMPL_STABLE_TRANSFORM_HPP

#include <compose/stable_transform.hpp>
#include <compose/transformed_operation.hpp>

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
stable_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  Args&&... args)
{
    return transformed_operation<
      detail::composed_op<OperationBody,
                          BOOST_ASIO_HANDLER_TYPE(CompletionToken, Signature),
                          Executor,
                          true>>{
      std::move(init.completion_handler), ex, std::forward<Args>(args)...};
}

} // namespace detail

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
  Args&&... args)
{
    return detail::stable_transform<OperationBody>(
      ex, init, std::forward<Args>(args)...);
}

template<typename Executor,
         typename CompletionToken,
         typename Signature,
         typename OperationBody>
auto
stable_transform(
  Executor const& ex,
  boost::asio::async_completion<CompletionToken, Signature>& init,
  OperationBody&& ob)
{
    return detail::stable_transform<Signature,
                                    detail::remove_cref_t<OperationBody>>(
      ex, init, std::forward<OperationBody>(ob));
}

} // namespace compose

#endif // COMPOSE_IMPL_STABLE_TRANSFORM_HPP
