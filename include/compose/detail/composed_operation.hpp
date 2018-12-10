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

#include <compose/detail/handler_storage.hpp>
#include <compose/yield_token.hpp>

#include <boost/asio/executor_work_guard.hpp>

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
template<class Executor>
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

private:
    Executor executor_;
};

template<class CompletionHandler, class IoExecutor>
using work_guard_t = typename std::conditional<
  std::is_same<
    boost::asio::associated_executor_t<CompletionHandler, IoExecutor>,
    IoExecutor>::value &&
    std::is_same<boost::asio::associated_executor_t<CompletionHandler>,
                 boost::asio::system_executor>::value,
  null_work_guard<IoExecutor>,
  boost::asio::executor_work_guard<
    boost::asio::associated_executor_t<CompletionHandler, IoExecutor>>>::type;

template<class Handler, class IoExecutor>
struct upcall_op
{
    template<class... Args>
    void operator()(Args&&... args)
    {
        auto const guard = std::move(guard_);
        (void)guard;
        upcall_(std::forward<Args>(args)...);
    }

    Handler upcall_;
    work_guard_t<Handler, IoExecutor> guard_;
};

template<class OperationBody, class Handler, class IoExecutor, bool stable>
class composed_op
{
public:
    template<class H, class... BodyArgs>
    explicit composed_op(H&& h, IoExecutor const& ex, BodyArgs&&... args)
      : op_storage_{
          upcall_op<Handler, IoExecutor>{std::move(h),
                                         work_guard_t<Handler, IoExecutor>{ex}},
          std::forward<BodyArgs>(args)...}
    {
    }

    explicit composed_op(yield_token<composed_op>&& token)
      : composed_op{std::move(token).release_operation()}
    {
    }

    template<class... Args>
    void operator()(Args&&... args)
    {
        (void)op_storage_.value()(yield_token<composed_op>{*this, true},
                                  std::forward<Args>(args)...);
    }

    template<class... Args>
    void run(Args&&... args)
    {
        (void)op_storage_.value()(yield_token<composed_op>{*this, false},
                                  std::forward<Args>(args)...);
    }

    template<class... Args>
    upcall_guard post_upcall(Args&&... args)
    {
        assert(op_storage_.has_value() &&
               "post_upcall must not be called on an invalid operation.");
        auto const ex = boost::asio::get_associated_executor(*this);
        auto const alloc = boost::asio::get_associated_allocator(*this);
        ex.post(op_storage_.release_bind(std::forward<Args>(args)...), alloc);
        return {};
    }

    template<class... Args>
    upcall_guard direct_upcall(Args&&... args)
    {
        assert(op_storage_.has_value() &&
               "direct_upcall must not be called on an invalid operation.");

        op_storage_.release_bind(std::forward<Args>(args)...)();
        return {};
    }

    template<class H, class E>
    friend class boost::asio::associated_executor;

    template<class H, class A>
    friend class boost::asio::associated_allocator;

private:
    detail::
      handler_storage<upcall_op<Handler, IoExecutor>, OperationBody, stable>
        op_storage_;
};

} // namespace detail

} // namespace compose

namespace boost
{
namespace asio
{

template<class OperationBody,
         class Handler,
         class IoExecutor,
         bool stable,
         class Ex>
class associated_executor<
  ::compose::detail::composed_op<OperationBody, Handler, IoExecutor, stable>,
  Ex>
{
public:
    using type = associated_executor_t<Handler, IoExecutor>;

    static type get(
      ::compose::detail::
        composed_op<OperationBody, Handler, IoExecutor, stable> const& op,
      Ex const& = Ex{})
    {
        return asio::get_associated_executor(
          op.op_storage_.handler().upcall_,
          op.op_storage_.handler().guard_.get_executor());
    }
};

template<class OperationBody,
         class Handler,
         class IoExecutor,
         bool stable,
         class A>
class associated_allocator<
  ::compose::detail::composed_op<OperationBody, Handler, IoExecutor, stable>,
  A>
{
public:
    using type = associated_allocator_t<Handler, A>;

    static type get(
      ::compose::detail::
        composed_op<OperationBody, Handler, IoExecutor, stable> const& op,
      A const& alloc = A{})
    {
        return asio::get_associated_allocator(op.op_storage_.handler().upcall_,
                                              alloc);
    }
};

} // namespace asio
} // namespace boost

#endif // COMPOSE_DETAIL_COMPOSED_OPERATION_HPP
