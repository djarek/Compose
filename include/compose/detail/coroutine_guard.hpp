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
#include <compose/detail/assume.hpp>

namespace compose
{

class coroutine;

namespace detail
{

class coroutine_guard
{
public:
    explicit coroutine_guard(coroutine& c) noexcept
      : coroutine_guard(&c)
    {
    }

    explicit coroutine_guard(coroutine* c) noexcept
      : ref_{c}
    {
    }

    coroutine_guard& operator=(int i)
    {
        ref_->state_ = i;
        release();
        return *this;
    }

    operator int() const
    {
        COMPOSE_ASSUME(ref_ != nullptr);
        return ref_->state_;
    }

    void release()
    {
        (void)ref_.release();
    }

private:
    struct completion_guard
    {
        void operator()(coroutine* c)
        {
            c->state_ = -1;
        }
    };

    std::unique_ptr<coroutine, completion_guard> ref_;
};

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_COROUTINE_HPP
