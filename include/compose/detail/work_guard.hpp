//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#ifndef COMPOSE_DETAIL_WORK_GUARD_HPP
#define COMPOSE_DETAIL_WORK_GUARD_HPP

#include <boost/asio/associated_executor.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include <type_traits>

namespace compose
{
namespace detail
{

/**
 * A work_guard-like class that explicitly does not call on_work*() functions of
 * the Executor. Only to be used if a CompletionHandler is associated with the
 * system executor (in which case a regular work guard performs redundant work
 * management and has a useless flag in it)
 */
template<typename Executor>
class null_work_guard
{
public:
    using executor_type = Executor;

    explicit constexpr null_work_guard(Executor const& ex) noexcept
      : executor_{ex}
    {
    }

    executor_type get_executor() const noexcept
    {
        return executor_;
    }

    bool owns_work() const = delete;

    void reset() = delete;

private:
    Executor executor_;
};

template<typename CompletionHandler, typename IoExecutor>
using work_guard_t = typename std::conditional<
  std::is_same<
    boost::asio::associated_executor_t<CompletionHandler, IoExecutor>,
    IoExecutor>::value &&
    std::is_same<boost::asio::associated_executor_t<CompletionHandler>,
                 boost::asio::system_executor>::value,
  null_work_guard<IoExecutor>,
  boost::asio::executor_work_guard<
    boost::asio::associated_executor_t<CompletionHandler, IoExecutor>>>::type;

template<typename CompletionHandler, typename IoExecutor>
auto
make_work_guard(CompletionHandler const& ch, IoExecutor const& ex)
  -> work_guard_t<CompletionHandler, IoExecutor>
{
    return work_guard_t<CompletionHandler, IoExecutor>{
      boost::asio::get_associated_executor(ch, ex)};
}

} // namespace detail
} // namespace compose

#endif // COMPOSE_DETAIL_WORK_GUARD_HPP
