//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_COROUTINE_OP_HPP
#define COMPOSE_DETAIL_COROUTINE_OP_HPP

#include <compose/yield_token.hpp>

namespace compose
{

namespace detail
{

template<typename Op>
class coroutine_op
{
public:
    using executor_type = typename Op::executor_type;

    using allocator_type = typename Op::allocator_type;

    template<typename DeducedOp, typename... BodyArgs>
    explicit coroutine_op(DeducedOp&& h, BodyArgs&&... args)
      : resume_{std::forward<DeducedOp>(h), std::forward<BodyArgs>(args)...}
    {
    }

    template<typename DeducedOp>
    explicit coroutine_op(yield_token<DeducedOp>&& token)
      : resume_{std::move(token).release_operation()}
    {
    }

    executor_type get_executor() const noexcept
    {
        return resume_.get_executor();
    }

    allocator_type get_allocator() const noexcept
    {
        return resume_.get_allocator();
    }

    template<typename... Args>
    void operator()(Args&&... args)
    {
        (void)op_storage_.value()(yield_token<coroutine_op&>{*this, true},
                                  std::forward<Args>(args)...);
    }

    template<typename... Args>
    void run(Args&&... args)
    {
        resume_.run(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void resume(Args&&... args)
    {
        resume_.run(std::forward<Args>(args)...);
    }

    template<typename... Args>
    upcall_guard post_upcall(Args&&... args)
    {
        return resume_(std::forward<Args>(args)...);
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
    Op resume_;
};

} // namespace detail

} // namespace compose

#endif // COMPOSE_DETAIL_COROUTINE_OP_HPP
