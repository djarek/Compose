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

namespace compose
{

struct async_test_tag_dispatch
{
    template<typename TimerType>
    struct tag_dispatch_op
    {
        struct timer_tag_t
        {
        };

        template<typename YieldToken>
        upcall_guard operator()(YieldToken yield_token)
        {
            return (*this)(std::move(yield_token),
                           timer_tag_t{},
                           boost::system::error_code{});
        }

        template<typename YieldToken>
        upcall_guard operator()(YieldToken yield_token,
                                timer_tag_t,
                                boost::system::error_code ec)
        {
            if (i_-- <= 0)
            {
                return std::move(yield_token).upcall(ec);
            }

            timer_.expires_from_now(duration_);
            return timer_.async_wait(
              bind_token(std::move(yield_token), timer_tag_t{}));
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
        auto op = unstable_transform<tag_dispatch_op<TimerType>>(
          timer.get_executor(), init, std::piecewise_construct, timer, d, i);
        std::move(op).run();

        return init.result.get();
    }
};

struct async_test_coroutine_termination
{
    struct coro_op
    {
        template<typename YieldToken>
        upcall_guard operator()(YieldToken&& yield_token,
                                boost::system::error_code ec = {})
        {
            COMPOSE_REENTER(coro_state_)
            {
            }

            BOOST_TEST(coro_state_.is_complete());
            return std::move(yield_token).upcall(ec);
        }
        compose::coroutine coro_state_{};
    };

    template<typename CompletionToken>
    auto operator()(boost::asio::steady_timer& t,
                    std::chrono::microseconds,
                    int,
                    CompletionToken&& tok)
      -> BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                       void(boost::system::error_code))
    {
        boost::asio::async_completion<CompletionToken,
                                      void(boost::system::error_code)>
          init{tok};
        auto op = unstable_transform(t.get_executor(), init, coro_op{});
        std::move(op).run();
        return init.result.get();
    }
};

struct async_test_copyable_coro
{
    template<typename TimerType>
    struct coro_op
    {

        template<typename YieldToken>
        upcall_guard operator()(YieldToken&& yield_token,
                                boost::system::error_code ec = {})
        {
            COMPOSE_REENTER(coro_state_)
            {
                if (i_ == 0)
                    COMPOSE_RETURN std::move(yield_token).upcall(ec);

                for (i_ = 0; i_ < 5; ++i_)
                {
                    timer_.expires_from_now(duration_);
                    COMPOSE_YIELD timer_.async_wait(std::move(yield_token));
                    if (ec)
                        COMPOSE_RETURN std::move(yield_token).direct_upcall(ec);
                }
                COMPOSE_RETURN std::move(yield_token).direct_upcall(ec);
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
        auto op = unstable_transform(
          timer.get_executor(), init, coro_op<TimerType>{timer, d, i});
        std::move(op).run();
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
    compose::test_composed_op<compose::async_test_tag_dispatch>();
    compose::test_composed_op<compose::async_test_copyable_coro>();
    compose::test_composed_op<compose::async_test_coroutine_termination>();

    return boost::report_errors();
}
