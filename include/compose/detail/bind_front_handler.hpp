//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_BIND_FRONT_HANDLER_HPP
#define COMPOSE_DETAIL_BIND_FRONT_HANDLER_HPP

#include <compose/detail/lean_tuple.hpp>
#include <compose/upcall_guard.hpp>

#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/async_result.hpp>

namespace compose
{

namespace detail
{

template<class Handler, class... Args>
struct bound_front_op
{
    template<class... Ts>
    void operator()(Ts&&... ts)
    {
        invoke_handler(boost::mp11::index_sequence_for<Args...>{},
                       std::forward<Ts>(ts)...);
    }

    Handler handler_;
    detail::lean_tuple<Args...> args_;

private:
    template<std::size_t... I, class... Ts>
    void invoke_handler(boost::mp11::index_sequence<I...>, Ts&&... ts)
    {
        handler_(detail::get<I>(std::move(args_))..., std::forward<Ts>(ts)...);
    }
};

template<class T>
using remove_cref_t =
  typename std::remove_const<typename std::remove_reference<T>::type>::type;

template<class Handler, class... Args>
auto
bind_front_handler(Handler&& h, Args&&... args)
  -> bound_front_op<remove_cref_t<Handler>, typename std::decay<Args>::type...>
{
    return bound_front_op<remove_cref_t<Handler>,
                          typename std::decay<Args>::type...>{
      std::forward<Handler>(h), {std::forward<Args>(args)...}};
}

} // namespace detail
} // namespace compose

namespace boost
{
namespace asio
{

template<class Handler, class... Args, class Signature>
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

template<class Handler, class... Args, class Ex>
class associated_executor<::compose::detail::bound_front_op<Handler, Args...>,
                          Ex>
{
public:
    using type = associated_executor_t<Handler, Ex>;

    static type get(
      ::compose::detail::bound_front_op<Handler, Args...> const& op,
      Ex const& ex = Ex{})
    {
        return asio::get_associated_executor(op.handler_, ex);
    }
};

template<class Handler, class... Args, class A>
class associated_allocator<::compose::detail::bound_front_op<Handler, Args...>,
                           A>
{
public:
    using type = associated_allocator_t<Handler, A>;

    static type get(
      ::compose::detail::bound_front_op<Handler, Args...> const& op,
      A const& alloc = A{})
    {
        return asio::get_associated_allocator(op.handler_, alloc);
    }
};

} // namespace asio
} // namespace boost

#endif // COMPOSE_DETAIL_BIND_FRONT_HANDLER_HPP
