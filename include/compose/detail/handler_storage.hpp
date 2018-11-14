//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_HANDLER_STORAGE_HPP
#define COMPOSE_DETAIL_HANDLER_STORAGE_HPP

#ifndef COMPOSE_NO_RECYCLING_ALLOCATOR
#include <boost/asio/detail/recycling_allocator.hpp>
#endif // COMPOSE_NO_RECYCLING_ALLOCATOR

#include <compose/detail/bind_handler_front.hpp>

#include <boost/core/exchange.hpp>
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

template<typename Handler, typename T, bool stable = false>
struct handler_storage
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
        std::unique_ptr<wrapper, decltype(deleter)> p{traits.allocate(alloc, 1),
                                                      deleter};
        traits.construct(alloc, p.get(), std::forward<Args>(args)...);
        constructed = true;
        ::new (static_cast<void*>(&handler_)) Handler{std::forward<H>(h)};
        wrapper_ = p.release();
    }

    handler_storage(handler_storage&& other) noexcept
    {
        wrapper_ = boost::exchange(other.wrapper_, nullptr);
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
        auto bound = detail::bind_handler_front(std::move(handler_),
                                                std::forward<Args>(args)...);
        typename std::allocator_traits<boost::asio::associated_allocator_t<
          Handler>>::template rebind_alloc<wrapper>
          alloc{boost::asio::get_associated_allocator(handler_)};
        std::allocator_traits<decltype(alloc)> traits;
        traits.destroy(alloc, wrapper_);
        traits.deallocate(alloc, wrapper_, 1);
        wrapper_ = nullptr;
        handler_.~Handler();
        return bound;
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
