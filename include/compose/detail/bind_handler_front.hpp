//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_BIND_HANDLER_FRONT_HPP
#define COMPOSE_DETAIL_BIND_HANDLER_FRONT_HPP

#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/mp11/integer_sequence.hpp>

#include <tuple>

namespace compose
{

namespace detail
{

template<typename CompletionHandler, typename... Args>
struct bound_front_op
{
    using executor_type = boost::asio::associated_executor_t<CompletionHandler>;
    using allocator_type =
      boost::asio::associated_allocator_t<CompletionHandler>;

    executor_type get_executor() const noexcept
    {
        return boost::asio::get_associated_executor(handler_);
    }

    allocator_type get_allocator() const noexcept
    {
        return boost::asio::get_associated_allocator(handler_);
    }

    template<typename... Ts>
    void operator()(Ts&&... ts)
    {
        (*this)(boost::mp11::index_sequence_for<Args...>{},
                std::forward<Ts>(ts)...);
    }

    CompletionHandler handler_;
    std::tuple<Args...> args_;

private:
    template<typename... Ts, std::size_t... I>
    void operator()(boost::mp11::index_sequence<I...>, Ts&&... ts)
    {
        handler_(std::get<I>(std::move(args_))..., std::forward<Ts>(ts)...);
    }
};

template<typename CompletionHandler, typename... Args>
auto
bind_handler_front(CompletionHandler&& ch, Args&&... args)
  -> decltype(bound_front_op<typename std::decay<CompletionHandler>::type,
                             typename std::decay<Args>::type...>{
    std::forward<CompletionHandler>(ch),
    std::make_tuple(std::forward<Args>(args)...)})
{
    return bound_front_op<typename std::decay<CompletionHandler>::type,
                          typename std::decay<Args>::type...>{
      std::forward<CompletionHandler>(ch),
      std::make_tuple(std::forward<Args>(args)...)};
}

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_BIND_HANDLER_FRONT_HPP
