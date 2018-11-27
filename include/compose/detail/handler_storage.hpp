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

#include <boost/asio/associated_allocator.hpp>
#include <compose/detail/bind_handler_front.hpp>
#include <compose/detail/lean_ptr.hpp>

#ifndef COMPOSE_NO_RECYCLING_ALLOCATOR
#include <boost/asio/detail/recycling_allocator.hpp>
#endif // COMPOSE_NO_RECYCLING_ALLOCATOR
#include <memory>

namespace compose
{
namespace detail
{

#ifndef COMPOSE_NO_RECYCLING_ALLOCATOR
using default_allocator = boost::asio::detail::recycling_allocator<void>;
#else
using default_allocator = std::allocator<void>;
#endif

template<typename T>
struct uniform_init_wrapper
{
    template<typename... Args>
    explicit uniform_init_wrapper(Args&&... args)
      : t_{std::forward<Args>(args)...}
    {
    }

    T t_;
};

template<typename Handler, typename Allocator>
struct deleter
{
    using value_type = typename Allocator::value_type;
    void operator()(value_type* p) const
    {
        std::allocator_traits<Allocator> traits;
        traits.destroy(alloc_, p);
        traits.deallocate(alloc_, p, 1);
        handler_.~Handler();
    }

    Allocator& alloc_;
    Handler& handler_;
};

template<typename Allocator>
struct conditional_deleter
{
    using value_type = typename Allocator::value_type;
    void operator()(value_type* p) const
    {
        std::allocator_traits<Allocator> traits;
        if (constructed_)
        {
            traits.destroy(alloc_, p);
        }

        traits.deallocate(alloc_, p, 1);
    }

    Allocator& alloc_;
    bool& constructed_;
};

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
    {
        return detail::bind_handler_front(std::move(handler_),
                                          std::forward<Args>(args)...);
    }
};

template<typename Handler, typename T>
class handler_storage<Handler, T, true>
{
    using wrapper = uniform_init_wrapper<T>;
    using allocator_type = typename std::allocator_traits<
      boost::asio::associated_allocator_t<Handler, default_allocator>>::
      template rebind_alloc<wrapper>;

public:
    template<typename H, typename... Args>
    explicit handler_storage(H&& h, Args&&... args)
    {
        allocator_type alloc{
          boost::asio::get_associated_allocator(h, default_allocator{})};
        std::allocator_traits<allocator_type> traits;
        bool constructed = false;

        detail::lean_ptr<wrapper, conditional_deleter<allocator_type>> p{
          traits.allocate(alloc, 1),
          conditional_deleter<allocator_type>{alloc, constructed}};
        traits.construct(alloc, p.t_, std::forward<Args>(args)...);
        constructed = true;
        ::new (static_cast<void*>(&handler_)) Handler{std::forward<H>(h)};
        wrapper_ = p.t_;
        p.t_ = nullptr;
    }

    handler_storage(handler_storage&& other) noexcept
    {
        wrapper_ = other.wrapper_;
        other.wrapper_ = nullptr;
        ::new (static_cast<void*>(&handler_))
          Handler{std::move(other.handler_)};
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
            deleter<Handler, allocator_type>{alloc, handler_}(wrapper_);
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
    {
        allocator_type alloc{
          boost::asio::get_associated_allocator(handler_, default_allocator{})};

        auto const del = deleter<Handler, allocator_type>{alloc, handler_};
        detail::lean_ptr<wrapper, decltype(del)> p{wrapper_, del};
        wrapper_ = nullptr;
        return detail::bind_handler_front(std::move(handler_),
                                          std::forward<Args>(args)...);
    }

private:
    union {
        Handler handler_;
    };

    wrapper* wrapper_;
};

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_HANDLER_STORAGE_HPP
