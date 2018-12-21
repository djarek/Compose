// std::forward<H>(h) rgstd : std::forward<Args>(args)
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_HANDLER_STORAGE_HPP
#define COMPOSE_DETAIL_HANDLER_STORAGE_HPP

#include <compose/detail/allocator_utils.hpp>
#include <compose/detail/bind_front_handler.hpp>
#include <compose/detail/lean_ptr.hpp>

namespace compose
{
namespace detail
{

template<typename Handler, typename T, bool stable>
struct handler_storage;

template<typename Handler, typename T>
struct handler_storage<Handler, T, false>
{
    Handler handler_;
    T t_;

    Handler& handler()
    {
        return handler_;
    }

    Handler const& handler() const
    {
        return handler_;
    }

    T& value()
    {
        return t_;
    }

    T const& value() const
    {
        return t_;
    }

    bool has_value() const noexcept
    {
        return true;
    }

    template<typename... Args>
    auto release_bind(Args&&... args)
      -> bound_front_op<Handler, typename std::decay<Args>::type...>
    {
        return {std::move(handler_), {std::forward<Args>(args)...}};
    }
};

template<typename Handler, typename T>
class handler_storage<Handler, T, true>
{
    using wrapper = uniform_init_wrapper<T>;
    using allocator_type = rebound_associated_alloc_t<Handler, wrapper>;

public:
    template<typename H, typename... Args>
    explicit handler_storage(H&& h, Args&&... args)
      : handler_{std::forward<H>(h)}
    {
        allocator_type alloc{
          boost::asio::get_associated_allocator(h, default_allocator{})};
        std::allocator_traits<allocator_type> traits;

        detail::lean_ptr<wrapper, deallocator<allocator_type>> p{
          traits.allocate(alloc, 1), deallocator<allocator_type>{alloc}};
        traits.construct(alloc, p.t_, std::forward<Args>(args)...);
        wrapper_ = p.t_;
        p.t_ = nullptr;
    }

    handler_storage(handler_storage&& other) noexcept
      : handler_{std::move(other.handler_)}
    {
        wrapper_ = other.wrapper_;
        other.wrapper_ = nullptr;
    }

    handler_storage(handler_storage const&) = delete;
    handler_storage& operator=(handler_storage&&) = delete;
    handler_storage& operator=(handler_storage const&) = delete;

    ~handler_storage()
    {
        if (has_value())
        {
            allocator_type alloc{boost::asio::get_associated_allocator(
              handler_, default_allocator{})};
            deleter<allocator_type>{alloc}(wrapper_);
        }
    }

    Handler& handler()
    {
        return handler_;
    }

    Handler const& handler() const
    {
        return handler_;
    }

    T& value()
    {
        return wrapper_->t_;
    }

    T const& value() const
    {
        return wrapper_->t_;
    }

    bool has_value() const noexcept
    {
        return wrapper_ != nullptr;
    }

    template<typename... Args>
    auto release_bind(Args&&... args)
      -> bound_front_op<Handler, typename std::decay<Args>::type...>
    {
        allocator_type alloc{
          boost::asio::get_associated_allocator(handler_, default_allocator{})};

        auto const del = deleter<allocator_type>{alloc};
        detail::lean_ptr<wrapper, decltype(del)> p{wrapper_, del};
        wrapper_ = nullptr;
        return {std::move(handler_), {std::forward<Args>(args)...}};
    }

private:
    Handler handler_;
    wrapper* wrapper_;
};

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_HANDLER_STORAGE_HPP
