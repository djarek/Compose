//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#include <compose/coroutine.hpp>
#include <compose/stable_transform.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/core/lightweight_test.hpp>

namespace compose_tests
{

template<typename TimerType>
struct noncopyable_op
{
    noncopyable_op(noncopyable_op const&) = delete;
    noncopyable_op(noncopyable_op&&) = delete;

    template<typename YieldToken>
    compose::upcall_guard operator()(YieldToken&& yield_token,
                                     boost::system::error_code ec = {})
    {
        COMPOSE_REENTER(coro_state_)
        {
            if (i_ == 0)
                return std::move(yield_token).upcall(ec);

            for (i_ = 0; i_ < 5; ++i_)
            {
                timer_.expires_from_now(duration_);
                COMPOSE_YIELD timer_.async_wait(std::move(yield_token));
                if (ec)
                    break;
            }

            return std::move(yield_token).direct_upcall(ec);
        }
    }

    TimerType& timer_;
    std::chrono::milliseconds duration_;
    int i_;
    compose::coroutine coro_state_{};
};

template<typename TimerType, typename CompletionToken>
auto
async_op(TimerType& timer,
         std::chrono::milliseconds d,
         int i,
         CompletionToken&& tok)
  -> BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                   void(boost::system::error_code))
{
    boost::asio::async_completion<CompletionToken,
                                  void(boost::system::error_code)>
      init{tok};
    auto op = compose::stable_transform<noncopyable_op<TimerType>>(
      timer.get_executor(), init, std::piecewise_construct, timer, d, i);
    std::move(op).run();
    return init.result.get();
}

} // namespace compose_tests

int
main()
{
    for (int i = 0; i < 2; ++i)
    {
        boost::asio::io_context ctx;
        boost::asio::steady_timer timer{ctx};
        int invoked = 0;
        boost::system::error_code ec;

        compose_tests::async_op(
          timer,
          std::chrono::milliseconds{1},
          i,
          [&invoked, &ec](boost::system::error_code ec_arg) {
              ec = ec_arg;
              ++invoked;
          });

        ctx.run();

        BOOST_TEST(invoked == 1);
        BOOST_TEST(!ec);
    }
    return boost::report_errors();
}
