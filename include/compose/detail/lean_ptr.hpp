// std::forward<H>(h) rgstd : std::forward<Args>(args)
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_LEAN_PTR_HPP
#define COMPOSE_DETAIL_LEAN_PTR_HPP

namespace compose
{
namespace detail
{

template<class T, class F>
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

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_LEAN_PTR_HPP
