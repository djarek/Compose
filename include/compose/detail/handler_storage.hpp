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

#include <compose/detail/bind_handler_front.hpp>

#include <boost/asio/associated_allocator.hpp>

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

template<typename T, typename F>
struct lean_ptr
{
    lean_ptr(lean_ptr&&) = delete;
    lean_ptr(lean_ptr const&) = delete;
    lean_ptr& operator=(lean_ptr&&) = delete;
    lean_ptr& operator=(lean_ptr const&) = delete;

    ~lean_ptr()
    {
        if (t_)
            f_(t_);
    }

    T* t_;
    F f_;
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
struct handler_storage<Handler, T, true>
{
    template<typename H, typename... Args>
    explicit handler_storage(H&& h, Args&&... args)
    {
        typename std::allocator_traits<
          boost::asio::associated_allocator_t<Handler, default_allocator>>::
          template rebind_alloc<wrapper>
            alloc{
              boost::asio::get_associated_allocator(h, default_allocator{})};
        std::allocator_traits<decltype(alloc)> traits;
        bool constructed = false;
        auto const deleter = [&](wrapper* w) {
            if (constructed)
            {
                traits.destroy(alloc, w);
            }

            traits.deallocate(alloc, w, 1);
        };

        detail::lean_ptr<wrapper, decltype(deleter)> p{
          traits.allocate(alloc, 1), deleter};
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
            typename std::allocator_traits<
              boost::asio::associated_allocator_t<Handler, default_allocator>>::
              template rebind_alloc<wrapper>
                alloc{boost::asio::get_associated_allocator(
                  handler_, default_allocator{})};
            std::allocator_traits<decltype(alloc)> traits;
            traits.destroy(alloc, wrapper_);
            traits.deallocate(alloc, wrapper_, 1);
            handler_.~Handler();
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
        typename std::allocator_traits<
          boost::asio::associated_allocator_t<Handler, default_allocator>>::
          template rebind_alloc<wrapper>
            alloc{boost::asio::get_associated_allocator(handler_,
                                                        default_allocator{})};

        auto const deleter = [&](wrapper* w) {
            wrapper_ = nullptr;
            std::allocator_traits<decltype(alloc)> traits;
            traits.destroy(alloc, w);
            traits.deallocate(alloc, w, 1);
            handler_.~Handler();
        };

        detail::lean_ptr<wrapper, decltype(deleter)> p{wrapper_, deleter};
        return detail::bind_handler_front(std::move(handler_),
                                          std::forward<Args>(args)...);
    }

private:
    struct wrapper
    {
        template<typename... Args>
        wrapper(Args&&... args)
          : t_{std::forward<Args>(args)...}
        {
        }

        T t_;
    };

    union {
        Handler handler_;
    };

    wrapper* wrapper_;
};

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_HANDLER_STORAGE_HPP
