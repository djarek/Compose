#include <compose/detail/lean_tuple.hpp>

#include <boost/core/lightweight_test.hpp>
#include <memory>

struct tag
{
};

int
main()
{
    struct empty
    {
    };
    struct explicit_constructible
    {
        explicit_constructible(std::nullptr_t)
          : i_(0)
        {
        }

        explicit explicit_constructible(int i)
          : i_(i)
        {
        }

        int i_;
    };

    BOOST_TEST(sizeof(compose::detail::lean_tuple<int, empty>) == sizeof(int));
    compose::detail::lean_tuple<explicit_constructible, int> t{nullptr, 42};
    BOOST_TEST(compose::detail::get<1>(t) == 42);
    BOOST_TEST(compose::detail::get<0>(t).i_ == 0);

    t = compose::detail::lean_tuple<explicit_constructible, int>{
      explicit_constructible(42), 43};
    BOOST_TEST(compose::detail::get<1>(t) == 43);
    BOOST_TEST(compose::detail::get<0>(t).i_ == 42);
    return boost::report_errors();
}
