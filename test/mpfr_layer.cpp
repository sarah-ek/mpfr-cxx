#define MPFR_CXX_DEBUG 1

#include "doctest.h"
#include <iostream>
#include "mpfr/mpfr.hpp"
#include <cassert>

template <typename Enable, typename... Args> struct valid_arg_checker_impl : std::false_type {};
template <typename... Args>
struct valid_arg_checker_impl<
    std::void_t<decltype(mpfr::handle_as_mpfr_t(std::declval<Args>()...))>,
    Args...> : std::true_type {};

template <typename... Args>
static constexpr bool valid_args = valid_arg_checker_impl<void, Args...>::value;

DOCTEST_TEST_CASE("out example") {
  /// [mpfr-layer-test begin]
#if 0
  /// for doc generation
#include "mpfr/mpfr.hpp"
#include <cassert>

  auto main() -> int {
  }
#endif
  using namespace mpfr;
  using scalar512_t = mp_float_t<digits2{512}>;
  using scalar1024_t = mp_float_t<digits2{1024}>;
  scalar512_t x; // uninitialized
  scalar1024_t y = scalar1024_t{2.0 + 1.0 / 1024};
  auto err_code = handle_as_mpfr_t(
      [](mpfr_ptr a, mpfr_srcptr b) {
        auto err = mpfr_set_str(a, "0.1111", 2, MPFR_RNDN); // set to 0b0.1111 == 0.9375
        mpfr_add(a, a, b, MPFR_RNDN);
        return err;
      },
      x,
      y);
  assert(err_code == 0);
  assert(x == scalar512_t{2 + 0.5 + 0.25 + 0.125 + 0.0625 + 1.0 / 1024});
  /// [mpfr-layer-test end]
  DOCTEST_CHECK(err_code == 0);
  DOCTEST_CHECK(x == scalar512_t{2 + 0.5 + 0.25 + 0.125 + 0.0625 + 1.0 / 1024});
}

using namespace mpfr;
using scalar_t = mp_float_t<digits10{1000}>;

DOCTEST_TEST_CASE("in") {
  scalar_t const x = 2;
  auto res = handle_as_mpfr_t(
      [](mpfr_srcptr a) {
        mpfr_t b;
        mpfr_t c;

        mpfr_init2(b, 64);
        mpfr_init2(c, 64);

        mpfr_set_ui(b, 4, MPFR_RNDN);
        mpfr_add(c, a, a, MPFR_RNDN);

        DOCTEST_CHECK(mpfr_equal_p(c, b) != 0);

        mpfr_clear(b);
        mpfr_clear(c);
        return 1;
      },
      x);
  DOCTEST_CHECK(res == 1);
}

DOCTEST_TEST_CASE("out") {
  scalar_t x;
  scalar_t y = 7;
  handle_as_mpfr_t([](mpfr_ptr a, mpfr_srcptr b) { mpfr_exp(a, b, MPFR_RNDN); }, x, y);
  DOCTEST_CHECK(x == exp(y));
}

DOCTEST_TEST_CASE("well-formedness static test") {
  auto fn1 = [](mpfr_srcptr, mpfr_ptr) {};
  auto fn2 = [](mpfr_srcptr, mpfr_ptr, int) {}; // invalid function
  struct {
    void operator()(mpfr_srcptr) {}
  } fn3; // non const call operator
  struct {
    void operator()(mpfr_srcptr) && {}
  } fn4; // only called if moved

  static_assert(valid_args<decltype(fn1), scalar_t, scalar_t>);
  static_assert(valid_args<decltype(fn1), scalar_t&, scalar_t>);
  static_assert(valid_args<decltype(fn1), scalar_t&&, scalar_t>);
  static_assert(not valid_args<decltype(fn1), scalar_t, scalar_t const>); // can't remove const
  static_assert(not valid_args<decltype(fn1), scalar_t, scalar_t const&>);
  static_assert(not valid_args<decltype(fn1), scalar_t, scalar_t const&&>);
  static_assert(not valid_args<decltype(fn1), int, scalar_t>); // can only pass mp_float<_>

  static_assert(not valid_args<decltype(fn2), scalar_t, scalar_t>);
  static_assert(not valid_args<decltype(fn2), scalar_t, scalar_t, int>);

  static_assert(not valid_args<decltype(fn3) const&, scalar_t>);
  static_assert(valid_args<decltype(fn3)&, scalar_t>);
  static_assert(valid_args<decltype(fn4)&&, scalar_t>);

  static_assert(not valid_args<decltype(fn4)&, scalar_t>);
  static_assert(valid_args<decltype(fn4)&&, scalar_t>);
}
