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

namespace compose
{
namespace detail
{
class coroutine_ref;
} // namespace detail

class coroutine
{
public:
    bool is_continuation() const
    {
        return state_ > 0;
    }

    friend detail::coroutine_ref;

private:
    int state_ = 0;
};

#ifdef __GNUG__
#define COMPOSE_ASSUME(expr)                                                   \
    do                                                                         \
    {                                                                          \
        assert(expr);                                                          \
        if (!(expr))                                                           \
            __builtin_unreachable();                                           \
    } while (0)
#else
#define COMPOSE_ASSUME(expr) assert(expr)
#endif

#define COMPOSE_REENTER(coroutine)                                             \
    switch (::compose::detail::coroutine_ref _coro_ref{coroutine})             \
    default:                                                                   \
        if (_coro_ref)                                                         \
        {                                                                      \
                                                                               \
            COMPOSE_ASSUME(false && "Corrupt coro state.");                    \
        }                                                                      \
        else /* fall-through */                                                \
        case 0:

#define COMPOSE_YIELD_IMPL(n)                                                  \
    for (_coro_ref = (n);;)                                                    \
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

} // namespace compose

#include <compose/detail/coroutine_guard.hpp>

#endif // COMPOSE_COROUTINE_HPP
