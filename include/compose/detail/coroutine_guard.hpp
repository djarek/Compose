//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_COROUTINE_HPP
#define COMPOSE_DETAIL_COROUTINE_HPP

#include <compose/coroutine.hpp>
namespace compose
{

class coroutine;

namespace detail
{

class coroutine_ref
{
public:
    explicit coroutine_ref(coroutine& c) noexcept
      : coro_(c)
    {
    }

    coroutine_ref(coroutine_ref&&) = delete;
    coroutine_ref(coroutine_ref const&) = delete;

    coroutine_ref& operator=(coroutine_ref&&) = delete;
    coroutine_ref& operator=(coroutine_ref const&) = delete;

    ~coroutine_ref() = default;

    coroutine_ref& operator=(int i)
    {
        coro_.state_ = i;
        return *this;
    }

    operator int() const
    {
        return coro_.state_;
    }

private:
    coroutine& coro_;
};

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_COROUTINE_HPP
