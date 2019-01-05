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
    boost::asio::executor_work_guard<IoExecutor> guard_;
};

template<class OperationBody, class Handler, class IoExecutor, bool stable>
class composed_op
{
public:
    template<class H, class... BodyArgs>
    explicit composed_op(H&& h, IoExecutor const& ex, BodyArgs&&... args)
      : op_storage_{upcall_op<Handler, IoExecutor>{
                      std::move(h),
                      boost::asio::make_work_guard<IoExecutor>(ex)},
                    std::forward<BodyArgs>(args)...}
    {
    }

    explicit composed_op(yield_token<composed_op> const& token)
      : composed_op{token.release_operation()}
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
    void post_upcall(Args&&... args)
    {
        assert(op_storage_.has_value() &&
               "post_upcall must not be called on an invalid operation.");
        auto const ex = boost::asio::get_associated_executor(
          *this, op_storage_.handler().guard_.get_executor());
        auto const alloc = boost::asio::get_associated_allocator(*this);
        ex.post(op_storage_.release_bind(std::forward<Args>(args)...), alloc);
    }

    template<class... Args>
    void direct_upcall(Args&&... args)
    {
        assert(op_storage_.has_value() &&
               "direct_upcall must not be called on an invalid operation.");

        op_storage_.release_bind(std::forward<Args>(args)...)();
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
        return associated_executor<Handler, IoExecutor>::get(
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
        return associated_allocator<Handler, A>::get(
          op.op_storage_.handler().upcall_, alloc);
    }
};

} // namespace asio
} // namespace boost

#endif // COMPOSE_DETAIL_COMPOSED_OPERATION_HPP
