//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//

#include <compose/unstable_transform.hpp>

#include <compose/bind_token.hpp>
#include <compose/coroutine.hpp>

#include <boost/asio/steady_timer.hpp>
#include <boost/core/lightweight_test.hpp>

namespace compose_tests
{

struct async_test_tag_dispatch
{
    template<typename TimerType>
    struct tag_dispatch_op
    {
        struct timer_tag_t
        {
        };

        template<typename Self>
        compose::upcall_guard operator()(compose::yield_token<Self> yield)
        {
            return (*this)(yield, timer_tag_t{}, boost::system::error_code{});
        }

        template<typename Self>
        compose::upcall_guard operator()(compose::yield_token<Self> yield,
                                         timer_tag_t,
                                         boost::system::error_code ec)
        {
            if (i_-- <= 0)
            {
                return yield.upcall(ec);
            }

            timer_.expires_from_now(duration_);
            return timer_.async_wait(compose::bind_token(yield, timer_tag_t{}));
        }

        TimerType& timer_;
        std::chrono::milliseconds duration_;
        int i_;
    };

    template<typename TimerType, typename CompletionToken>
    auto operator()(TimerType& timer,
                    std::chrono::milliseconds d,
                    int i,
                    CompletionToken&& tok)
      -> BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                       void(boost::system::error_code))
    {
        boost::asio::async_completion<CompletionToken,
                                      void(boost::system::error_code)>
          init{tok};

        compose::unstable_transform<tag_dispatch_op<TimerType>>(
          timer.get_executor(), init, std::piecewise_construct, timer, d, i)
          .run();

        return init.result.get();
    }
};

struct async_test_copyable_coro
{
    template<typename TimerType>
    struct coro_op
    {

        template<typename Self>
        compose::upcall_guard operator()(compose::yield_token<Self> yield,
                                         boost::system::error_code ec = {})
        {
            COMPOSE_REENTER(coro_state_)
            {
                if (i_ == 0)
                    return yield.upcall(ec);

                for (i_ = 0; i_ < 5; ++i_)
                {
                    timer_.expires_from_now(duration_);
                    COMPOSE_YIELD timer_.async_wait(yield);
                    if (ec)
                        return yield.direct_upcall(ec);
                }
                return yield.direct_upcall(ec);
            }
        }

        TimerType& timer_;
        std::chrono::milliseconds duration_;
        int i_;
        compose::coroutine coro_state_{};
    };

    template<typename TimerType, typename CompletionToken>
    auto operator()(TimerType& timer,
                    std::chrono::milliseconds d,
                    int i,
                    CompletionToken&& tok)
      -> BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                       void(boost::system::error_code))
    {
        boost::asio::async_completion<CompletionToken,
                                      void(boost::system::error_code)>
          init{tok};
        auto op = compose::unstable_transform(
          timer.get_executor(), init, coro_op<TimerType>{timer, d, i});
        op.run();
        return init.result.get();
    }
};

template<typename AsyncOpType>
void
test_composed_op()
{
    for (int i = 0; i < 2; ++i)
    {
        boost::asio::io_context ctx;
        boost::asio::steady_timer timer{ctx};
        int invoked = 0;
        boost::system::error_code ec;
        AsyncOpType async_op;

        async_op(timer,
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
}

} // namespace compose

int
main()
{
    compose_tests::test_composed_op<compose_tests::async_test_tag_dispatch>();
    compose_tests::test_composed_op<compose_tests::async_test_copyable_coro>();

    return boost::report_errors();
}
