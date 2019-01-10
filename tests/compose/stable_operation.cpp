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

#include <boost/asio/write.hpp>

namespace compose_tests
{

template<class TimerType>
struct noncopyable_op
{
    noncopyable_op(noncopyable_op const&) = delete;
    noncopyable_op(noncopyable_op&&) = delete;

    template<class Self>
    compose::upcall_guard operator()(compose::yield_token<Self> yield,
                                     boost::system::error_code ec = {})
    {
        if (ec || i_-- == 0)
            return yield.direct_upcall(ec);

        timer_.expires_from_now(duration_);
        return timer_.async_wait(yield);
    }

    TimerType& timer_;
    std::chrono::milliseconds duration_;
    unsigned i_;
};

template<class TimerType, class CompletionToken>
auto
async_op(TimerType& timer,
         std::chrono::milliseconds d,
         unsigned i,
         CompletionToken&& tok)
  -> BOOST_ASIO_INITFN_RESULT_TYPE(CompletionToken,
                                   void(boost::system::error_code))
{
    boost::asio::async_completion<CompletionToken,
                                  void(boost::system::error_code)>
      init{tok};

    if (i == 0)
    {
        i = 1;
        d = {};
    }

    compose::stable_transform<noncopyable_op<TimerType>>(
      timer.get_executor(), init, std::piecewise_construct, timer, d, i)
      .run();
    return init.result.get();
}

namespace net = boost::asio;
using boost::system::error_code;

template<class Context, class Token>
auto
async_post(Context& ctx, Token&& token)
{
    auto ex = ctx.get_executor();
    net::async_completion<Token, void()> completion(token);
    compose::stable_transform(
      ex, completion, [](auto yield) { return yield.upcall(); })
      .run();
    return completion.result.get();
}

} // namespace compose_tests

int
main()
{
    for (unsigned i = 0u; i < 2u; ++i)
    {
        boost::asio::io_context ctx;
        boost::asio::steady_timer timer{ctx};
        int invoked = 0;
        boost::system::error_code ec;

        compose_tests::async_op(
          timer,
          std::chrono::milliseconds{100},
          i,
          [&invoked, &ec](boost::system::error_code ec_arg) {
              ec = ec_arg;
              ++invoked;
          });

        ctx.run();

        BOOST_TEST(invoked == 1);
        BOOST_TEST(!ec);
    }

    {
        boost::asio::io_context ctx;
        int invoked = 0;

        compose_tests::async_post(ctx, [&invoked]() { ++invoked; });

        ctx.run();

        BOOST_TEST(invoked == 1);
    }

    return boost::report_errors();
}
