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

#include <compose/detail/lean_tuple.hpp>

namespace compose
{

namespace detail
{

template<typename Handler, typename... Args>
struct bound_front_op
{
    using executor_type = typename Handler::executor_type;
    using allocator_type = typename Handler::allocator_type;

    executor_type get_executor() const noexcept
    {
        return handler_.get_executor();
    }

    allocator_type get_allocator() const noexcept
    {
        return handler_.get_allocator();
    }

    template<typename... Ts>
    void operator()(Ts&&... ts)
    {
        invoke_handler(boost::mp11::index_sequence_for<Args...>{},
                       std::forward<Ts>(ts)...);
    }

    Handler handler_;
    detail::lean_tuple<Args...> args_;

private:
    template<std::size_t... I, typename... Ts>
    void invoke_handler(boost::mp11::index_sequence<I...>, Ts&&... ts)
    {
        handler_(detail::get<I>(std::move(args_))..., std::forward<Ts>(ts)...);
    }
};

template<typename T>
using remove_cref_t =
  typename std::remove_const<typename std::remove_reference<T>::type>::type;

template<typename Handler, typename... Args>
auto
bind_handler_front(Handler&& h, Args&&... args)
  -> bound_front_op<remove_cref_t<Handler>, remove_cref_t<Args>...>
{
    return bound_front_op<remove_cref_t<Handler>, remove_cref_t<Args>...>{
      std::forward<Handler>(h),
      detail::lean_tuple<remove_cref_t<Args>...>{std::forward<Args>(args)...}};
}

} // namespace detail
} // namespace compose

namespace boost
{
namespace asio
{

template<typename Handler, typename... Args, typename Signature>
class async_result<compose::detail::bound_front_op<Handler, Args...>, Signature>
{
public:
    using return_type = compose::upcall_guard;
    using completion_handler_type =
      compose::detail::bound_front_op<Handler, Args...>;

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

#endif // COMPOSE_DETAIL_BIND_HANDLER_FRONT_HPP
