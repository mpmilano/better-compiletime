
#include "convert.hpp"
#include "instance.hpp"
#include "specification.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <boost/pfr/precise.hpp>
#include <iostream>
#include <type_traits>

namespace compile_time {
namespace client {
struct D {
  int five;
};
struct C {
  int four;
};
struct B {
  int three;
};
struct A {
  int one;
  specification::list<int> two;
  B three;
};

static_assert(boost::pfr::tuple_size<A>::value == 3,
              "sanity check pfr library works");
} // namespace client
// add things to the specification -- types to allow raw, or explicit overrides
// of how types should be converted to instances.
// Also user_definitions of converted classes.
namespace specification {
template <> struct convert_to_instance<client::C> { using type = client::C; };
template <> struct user_definition<client::D> : public client::D {};
} // namespace specification
} // namespace compile_time

using namespace compile_time;

constexpr auto try_me() {
  value::convert_to_instance_t<client::B> b{};
  b.match([](int &three) constexpr { three = 3; });
  return b;
}

constexpr auto try_harder() {
  value::convert_to_instance_t<client::A> a{};
  a.match([](int &one, auto & /*two*/,
             value::instance<client::B> &three) constexpr {
    one = 1;
    three.match([](int &three) constexpr { three = 5; });
  });
  return a;
}

constexpr auto try_3() {
  value::convert_to_instance_t<client::C> c{};
  // C can't match
  // c.match([](auto&&... a) constexpr { return (f(a) + ...).unwrap(); });
  // but can ues its fields
  c.four = 4;
  return c;
}

constexpr auto try_4() {
  value::convert_to_instance_t<client::D> d{};
  // D can match
  d.match([](int &five) constexpr { five = 5; });
  // and also use its fields
  d.five++;
  return d;
}

struct boring_top {
  constexpr boring_top() = default;
  specification::top_pointer p{};
};
struct boringer_body {
  constexpr boringer_body() = default;
  std::size_t i{0};
};

/* namespace compile_time::specification{
    template<> struct user_definition<boring_top> : public boring_top {constexpr
user_definition() = default;}; template<> struct user_definition<boringer_body>
: public boringer_body {constexpr user_definition() = default;};
}*/

using default_allocator =
    ctctx::Allocator<boring_top, boring_top, boringer_body>;
struct default_holder {
  static const constexpr default_allocator allocator{};
  constexpr default_holder() = default;
};

constexpr auto try_with_allocator_sub() {
  default_allocator a;
  auto ref = ctctx::allocate<boringer_body>(a);
  ref.get(a).match([](auto &i) constexpr { i = 5; });
  a.top.match([&](auto &p) constexpr { p = erased_ref{std::move(ref), a}; });
  return a;
}

struct holder_for_try_with_allocator {
  static const constexpr default_allocator allocator{try_with_allocator_sub()};
};
template <typename _holder> struct holder_for_try_with_allocator_F {
  using holder = _holder;
  static constexpr const DECT(holder::allocator.top) &value =
      holder::allocator.top;
  constexpr const auto &operator()() const { return value; }
  constexpr holder_for_try_with_allocator_F() = default;
};

struct try_with_allcator_str {
  using F = holder_for_try_with_allocator_F<holder_for_try_with_allocator>;
  using holder = typename F::holder;
  // return compile_time_context<holder, boring_top, boringer_body>::template
  // convert_to_type<F>{};
  using step1 = compile_time_context<holder>::template convert_to_type<F>;
  struct step2 {
    static const constexpr default_allocator allocator{
        convert_to_value<default_allocator, step1>()};
  };
  using step2a = holder_for_try_with_allocator_F<step2>;
};

constexpr auto try_with_allocator() {
  return compile_time_context<typename try_with_allcator_str::holder>::
      template convert_to_type<typename try_with_allcator_str::step2a>{};
};

int main() {

  { // debug copy constructors
    default_allocator a;
    auto ref = ctctx::allocate<boringer_body>(a);
    ref.get(a).match([](auto &i) constexpr { i = 5; });
    auto copy_of_boringer_pre_move = ref.get(a);
    a.top.match([&](auto &p) constexpr { p = erased_ref{std::move(ref), a}; });
    auto copy_of_boringer_post_move = ref.get(a);
    auto copy_of_top = a.top;
    (void)copy_of_boringer_post_move;
    (void)copy_of_boringer_pre_move;
    (void)copy_of_top;
    std::cout << "break here" << std::endl;
  }

  auto &ref =
      holder_for_try_with_allocator_F<holder_for_try_with_allocator>{}();
  (void)ref;
  constexpr value::instance<client::B> b = try_me();
  constexpr auto three = b.match([](auto &&three) constexpr { return three; });
  static_assert(three == 3);
  constexpr auto a = try_harder();
  constexpr auto one = a.match([](const int &one, auto &&...) { return one; });
  static_assert(one == 1);
  constexpr auto five = a.match([](
      auto &&, auto &&, const value::instance<client::B> &three) constexpr {
    return three.match([](const int &three) constexpr { return three; });
  });
  static_assert(five == 5);
  constexpr auto c = try_3();
  constexpr auto four = c.four;
  constexpr auto d = try_4();
  constexpr auto six = d.five;
  static_assert(six == 6);
  std::cout << three << std::endl;
  std::cout << one << std::endl;
  std::cout << five << std::endl;
  std::cout << four << std::endl;
  std::cout << six << std::endl;
  struct_wrap(wrapped_harder, try_harder());
  using as_type = typename compile_time_context<
      default_holder>::template convert_to_type<wrapped_harder>;
  // quash warning
  static_assert(!std::is_arithmetic_v<as_type>);

  constexpr const auto with_allocator = try_with_allocator();
  using F = holder_for_try_with_allocator_F<holder_for_try_with_allocator>;
  using holder = typename F::holder;
  compile_time_context<holder>::template convert_to_type_f<F>();
  // try_with_allcator_str::step1::print();
  // with_allocator.print();
  (void)with_allocator;
}