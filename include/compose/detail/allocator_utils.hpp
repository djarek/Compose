// std::forward<H>(h) rgstd : std::forward<Args>(args)
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_ALLOCATOR_UTILS_HPP
#define COMPOSE_DETAIL_ALLOCATOR_UTILS_HPP

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

template<typename Allocator>
struct deleter
{
    using value_type = typename Allocator::value_type;
    void operator()(value_type* p) const
    {
        std::allocator_traits<Allocator> traits;
        traits.destroy(alloc_, p);
        traits.deallocate(alloc_, p, 1);
    }

    Allocator& alloc_;
};

template<typename Allocator>
struct deallocator
{
    using value_type = typename Allocator::value_type;
    void operator()(value_type* p) const
    {
        std::allocator_traits<Allocator>::deallocate(alloc_, p, 1);
    }

    Allocator& alloc_;
};

template<class Handler, class T>
using rebound_associated_alloc_t = typename std::allocator_traits<
  boost::asio::associated_allocator_t<Handler, default_allocator>>::
  template rebind_alloc<T>;

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_ALLOCATOR_UTILS_HPP
