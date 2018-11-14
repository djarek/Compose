//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_COROUTINE_HPP
#define COMPOSE_COROUTINE_HPP

#include <compose/detail/assume.hpp>

namespace compose
{
namespace detail
{
class coroutine_guard;
} // namespace detail

class coroutine
{
public:
    bool is_complete() const
    {
        return state_ < 0;
    }

    bool is_continuation() const
    {
        return state_ > 0;
    }

    friend detail::coroutine_guard;

private:
    int state_ = 0;
};

#define COMPOSE_REENTER(coroutine)                                             \
    switch (detail::coroutine_guard _coro_guard{coroutine})                    \
    case -1:                                                                   \
        if (_coro_guard)                                                       \
        {                                                                      \
            default:                                                           \
                COMPOSE_ASSUME(false && "Corrupt coro state.");                \
        }                                                                      \
        else /* fall-through */                                                \
        case 0:

#define COMPOSE_YIELD_IMPL(n)                                                  \
    for (_coro_guard = (n);;)                                                  \
        /* fall-through */                                                     \
        if (false)                                                             \
        {                                                                      \
            static_assert((n) <= std::numeric_limits<int>::max(),              \
                          "Label index exceeded maximal value.");              \
            case (n):;                                                         \
                break;                                                         \
        }                                                                      \
        else                                                                   \
            return

#define COMPOSE_YIELD COMPOSE_YIELD_IMPL(__LINE__)

#define COMPOSE_RETURN                                                         \
    for (_coro_guard.release();;)                                              \
    return

} // namespace compose

#include <compose/detail/coroutine_guard.hpp>

#endif // COMPOSE_COROUTINE_HPP
