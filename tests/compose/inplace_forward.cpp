//
// Copyright (c) 2018 Damian Jarek (damian dot jarek93 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/djarek/compose
//
#if __cplusplus >= 201703L
#include <compose/stable_inplace_transform.hpp>
#include <compose/yield_token.hpp>

#include <boost/asio/steady_timer.hpp>

#include <boost/core/lightweight_test.hpp>

namespace compose
{

struct parser
{
    parser() = default;

    parser(parser const&) = delete;
    parser(parser&&) = delete;

    parser operator=(parser const&) = delete;
    parser operator=(parser&&) = delete;

    ~parser() = default;
};

template<typename CompletionToken, typename Duration>
auto
async_inplace_op(boost::asio::steady_timer& timer,
                 Duration d,
                 int n,
                 CompletionToken&& ct)
{
    boost::asio::async_completion<CompletionToken,
                                  void(boost::system::error_code)>
      init{ct};

    auto op = stable_inplace_transform(
      timer.get_executor(),
      init,
      COMPOSE_INPLACE_FWD(
        [&timer, d, i = 0, n, p = parser{}](
          auto&& yield_token, boost::system::error_code ec = {}) mutable {
            if (i++ == n)
                return std::move(yield_token).upcall(ec);
            timer.expires_from_now(d);
            return timer.async_wait(std::move(yield_token));
        }));
    std::move(op).run();

    return init.result.get();
}

} // namespace compose

int
main()
{
    boost::asio::io_context ctx;
    boost::asio::steady_timer timer{ctx};
    int invoked = 0;
    boost::system::error_code ec;

    compose::async_inplace_op(
      timer,
      std::chrono::milliseconds{1},
      3,
      [&invoked, &ec](boost::system::error_code ec_arg) {
          ec = ec_arg;
          ++invoked;
      });

    ctx.run();

    BOOST_TEST(invoked == 1);
    BOOST_TEST(!ec);

    return boost::report_errors();
}
#else
#include <boost/core/lightweight_test.hpp>

int
main()
{
    return boost::report_errors();
}
#endif
