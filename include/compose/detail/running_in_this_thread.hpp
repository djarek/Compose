//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_RUNNING_IN_THIS_THREAD_HPP
#define COMPOSE_DETAIL_RUNNING_IN_THIS_THREAD_HPP

namespace compose
{
namespace detail
{

template<typename Executor>
auto
running_in_this_thread(Executor const& ex, decltype(nullptr))
  -> decltype(ex.running_in_this_thread())
{
    return ex.running_in_this_thread();
}

template<typename Executor>
bool
running_in_this_thread(Executor const&, ...)
{
    return true;
}

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_RUNNING_IN_THIS_THREAD_HPP
