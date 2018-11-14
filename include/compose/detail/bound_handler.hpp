//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_BOUND_HANDLER_HPP
#define COMPOSE_DETAIL_BOUND_HANDLER_HPP

#include <compose/upcall_guard.hpp>

#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/associated_executor.hpp>

namespace compose
{

namespace detail
{

template<typename T>
struct bound_handler
{
    using executor_type = boost::asio::associated_executor_t<T>;
    using allocator_type = boost::asio::associated_allocator_t<T>;

    executor_type get_executor() const noexcept
    {
        return boost::asio::get_associated_executor(t_);
    }

    allocator_type get_allocator() const noexcept
    {
        return boost::asio::get_associated_allocator(t_);
    }

    template<typename... Args>
    void operator()(Args&&... args)
    {
        return t_(std::forward<Args>(args)...);
    }

    T t_;
};

} // namespace detail
} // namespace compose

namespace boost
{
namespace asio
{

template<typename Handler, typename Signature>
class async_result<compose::detail::bound_handler<Handler>, Signature>
{
public:
    using return_type = compose::upcall_guard;
    using completion_handler_type = compose::detail::bound_handler<Handler>;

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

#endif // COMPOSE_DETAIL_BOUND_HANDLER_HPP
