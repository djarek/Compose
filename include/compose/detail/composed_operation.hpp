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

#include <boost/asio/post.hpp>
#include <compose/detail/bind_handler_front.hpp>
#include <compose/detail/handler_storage.hpp>

namespace compose
{

namespace detail
{

/**
 * A work_guard-like class that explicitly does not call on_work*() functions of
 * the Executor. Only to be used if a CompletionHandler is associated with the
 * system executor (in which case a regular work guard performs redundant work
 * management and has a useless flag in it)
 */
template<typename Executor>
class null_work_guard
{
public:
    using executor_type = Executor;

    explicit constexpr null_work_guard(Executor const& ex) noexcept
      : executor_{ex}
    {
    }

    executor_type get_executor() const noexcept
    {
        return executor_;
    }

    bool owns_work() const = delete;

    void reset() = delete;

private:
    Executor executor_;
};

template<typename CompletionHandler, typename IoExecutor>
using work_guard_t = typename std::conditional<
  std::is_same<
    boost::asio::associated_executor_t<CompletionHandler, IoExecutor>,
    IoExecutor>::value &&
    std::is_same<boost::asio::associated_executor_t<CompletionHandler>,
                 boost::asio::system_executor>::value,
  null_work_guard<IoExecutor>,
  boost::asio::executor_work_guard<
    boost::asio::associated_executor_t<CompletionHandler, IoExecutor>>>::type;

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
        handler_(std::forward<Args>(args)...);
    }

private:
    CompletionHandler handler_;
    work_guard_t<CompletionHandler, IoExecutor> guard_;
};

template<typename OperationBody,
         typename Handler,
         typename Executor,
         bool stable>
class composed_op
{
    using op_t = upcall_op<Handler, Executor>;

public:
    using executor_type = typename op_t::executor_type;
    using allocator_type = typename op_t::allocator_type;

    template<typename H, typename... BodyArgs>
    explicit composed_op(H&& h, Executor const& ex, BodyArgs&&... args)
      : op_storage_{op_t{std::move(h), ex}, std::forward<BodyArgs>(args)...}
    {
    }

    explicit composed_op(yield_token<composed_op>&& token)
      : composed_op{std::move(token).release_operation()}
    {
    }

    executor_type get_executor() const noexcept
    {
        return op_storage_.handler().get_executor();
    }

    allocator_type get_allocator() const noexcept
    {
        return op_storage_.handler().get_allocator();
    }

    template<typename... Args>
    void operator()(Args&&... args)
    {
        (void)op_storage_.value()(yield_token<composed_op>{*this, true},
                                  std::forward<Args>(args)...);
    }

    template<typename... Args>
    void run(Args&&... args)
    {
        (void)op_storage_.value()(yield_token<composed_op>{*this, false},
                                  std::forward<Args>(args)...);
    }

    template<typename... Args>
    upcall_guard post_upcall(Args&&... args)
    {
        assert(op_storage_.has_value() &&
               "post_upcall must not be called on an invalid operation.");
        auto const ex = op_storage_.handler().get_io_executor();
        return boost::asio::post(
          ex, op_storage_.release_bind(std::forward<Args>(args)...));
    }

    template<typename... Args>
    upcall_guard direct_upcall(Args&&... args)
    {
        assert(op_storage_.has_value() &&
               "direct_upcall must not be called on an invalid operation.");

        op_storage_.release_bind(std::forward<Args>(args)...)();
        return {};
    }

private:
    detail::handler_storage<op_t, OperationBody, stable> op_storage_;
};

} // namespace detail

} // namespace compose

#endif // COMPOSE_DETAIL_COMPOSED_OPERATION_HPP
