//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_COMPOSED_OPERATION_HPP
#define COMPOSE_DETAIL_COMPOSED_OPERATION_HPP

#include <compose/yield_token.hpp>

#include <compose/detail/bind_handler_front.hpp>
#include <compose/detail/handler_storage.hpp>
#include <compose/detail/work_guard.hpp>

#include <boost/asio/post.hpp>

namespace compose
{

namespace detail
{

template<typename CompletionHandler, typename IoExecutor>
struct upcall_op
{
    using executor_type =
      boost::asio::associated_executor_t<CompletionHandler, IoExecutor>;
    using allocator_type =
      boost::asio::associated_allocator_t<CompletionHandler>;

    using io_executor_type = IoExecutor;

    template<typename Handler>
    upcall_op(Handler&& h, IoExecutor const& ex)
      : handler_{std::forward<Handler>(h)}
      , guard_{ex}
    {
    }

    executor_type get_executor() const noexcept
    {
        return boost::asio::get_associated_executor(handler_,
                                                    guard_.get_executor());
    }

    IoExecutor get_io_executor() const noexcept
    {
        return guard_.get_executor();
    }

    allocator_type get_allocator() const noexcept
    {
        return boost::asio::get_associated_allocator(handler_);
    }

    template<typename... Args>
    void operator()(Args&&... args)
    {
        auto const guard = std::move(guard_);
        (void)guard;
        return handler_(std::forward<Args>(args)...);
    }

private:
    CompletionHandler handler_;
    work_guard_t<CompletionHandler, IoExecutor> guard_;
};

template<typename OperationBody, typename Handler, bool stable>
class composed_op
{
public:
    using executor_type = boost::asio::associated_executor_t<Handler>;

    using allocator_type = boost::asio::associated_allocator_t<Handler>;

    template<typename... BodyArgs>
    explicit composed_op(Handler&& h, BodyArgs&&... args)
      : op_storage_{std::move(h), std::forward<BodyArgs>(args)...}
    {
    }

    template<typename DeducedOp>
    explicit composed_op(yield_token<DeducedOp>&& token)
      : composed_op{std::move(token).release_operation()}
    {
    }

    executor_type get_executor() const noexcept
    {
        return boost::asio::get_associated_executor(
          op_storage_.handler(), op_storage_.handler().get_io_executor());
    }

    allocator_type get_allocator() const noexcept
    {
        return boost::asio::get_associated_allocator(op_storage_.handler());
    }

    template<typename... Args>
    void operator()(Args&&... args)
    {
        (void)op_storage_.value()(yield_token<composed_op&>{*this, true},
                                  std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    void operator()(yield_token<T>&& tok, Args&&... args)
    {
        (void)op_storage_.value()(std::move(tok), std::forward<Args>(args)...);
    }

    template<typename... Args>
    void run(Args&&... args)
    {
        (void)op_storage_.value()(yield_token<composed_op&>{*this, false},
                                  std::forward<Args>(args)...);
    }

    template<typename... Args>
    upcall_guard post_upcall(Args&&... args)
    {
        BOOST_ASSERT(op_storage_.has_value() &&
                     "post_upcall must not be called on an invalid operation.");

        auto const ex = op_storage_.handler().get_io_executor();
        boost::asio::post(
          ex, op_storage_.release_bind(std::forward<Args>(args)...));
        return upcall_guard{};
    }

    template<typename... Args>
    upcall_guard direct_upcall(Args&&... args)
    {
        BOOST_ASSERT(
          op_storage_.has_value() &&
          "direct_upcall must not be called on an invalid operation.");

        op_storage_.release_bind(std::forward<Args>(args)...)();

        return upcall_guard{};
    }

private:
    detail::handler_storage<Handler, OperationBody, stable> op_storage_;
};

} // namespace detail

} // namespace compose

#endif // COMPOSE_DETAIL_COMPOSED_OPERATION_HPP
