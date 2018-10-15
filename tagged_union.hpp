#pragma once
#include <cassert>
#include <cstddef>
#include <type_traits>

namespace compile_time {
template <std::size_t offset, typename... T> struct _tagged_union;

template <std::size_t offset, typename A> struct _tagged_union<offset, A> {
  constexpr _tagged_union() = default;
  A data{};
  constexpr A &get(A const *const) { return data; }
  constexpr A &get(std::integral_constant<std::size_t, offset> const *const) {
    return data;
  }

  template <typename V> constexpr A &reset(A const *const, V &&value) {
    data = value;
    return data;
  }
};

template <std::size_t offset, typename A, typename B, typename... rest>
struct _tagged_union<offset, A, B, rest...> {
  constexpr _tagged_union() = default;
  union union_t {
    A fst;
    B snd;
    _tagged_union<offset + 2, rest...> rst;
  };
  enum class active { _A, _B, Rst };
  active active_member = active::Rst;
  union_t data = {.rst = {_tagged_union<offset + 2, rest...>{}}};
  constexpr A &get(A const *const) { return data.fst; }
  constexpr B &get(B const *const) { return data.snd; }
  constexpr A &get(std::integral_constant<std::size_t, offset> const *const) {
    return get((A *)nullptr);
  }
  constexpr B &
  get(std::integral_constant<std::size_t, offset + 1> const *const) {
    return get((B *)nullptr);
  }
  template <typename T> constexpr T &get(T const *const e) {
    return data.rst.get(e);
  }

  constexpr A &reset(A const *const) {
    if (active_member != active::_A) {
      active_member = active::_A;
      data = union_t{.fst = A{}};
    }
    return data.fst;
  }
  constexpr B &reset(B const *const) {
    if (active_member != active::_B) {
      active_member = active::_B;
      data = union_t{.snd = B{}};
    }
    return data.snd;
  }
  template <typename T> constexpr T &reset(T const *const e) {
    if (active_member != active::Rst) {
      active_member = active::Rst;
      data = union_t{ .rst = _tagged_union<offset + 2, rest...>{}};
    }
    return data.rst.reset(e);
  }
};

template <std::size_t init, typename T, typename T2, typename... seq>
constexpr std::size_t index_translate_f() {
  if constexpr (std::is_same<T, T2>::value)
    return init;
  else
    return index_translate_f<init + 1, T, seq...>();
}
template <typename T, typename... seq>
constexpr inline std::size_t
    index_translate = index_translate_f<0, T, seq...>();

template <typename... T> struct tagged_union {
  constexpr tagged_union() = default;
  static_assert(sizeof...(T) >= 1, "Error: tagged union minimum size is 1.");
  _tagged_union<true, T...> data{};
  std::size_t active_member = sizeof...(T);

  template <typename T2> constexpr bool is_active() const {
    return active_member == index_translate<T2, T...>;
  }

  template <typename T2> constexpr T2 &get() {
    assert(is_active<T2>());
    return data.get((T2 *)nullptr);
  }

  template <std::size_t s> constexpr decltype(auto) get() {
    return get((std::integral_constant<std::size_t, s> *)nullptr);
  }

  template <typename T2> constexpr T2 &reset() {
    active_member = index_translate<T2, T...>;
    return data.reset((T2 *)nullptr);
  }

  /*
    template <typename F> constexpr decltype(auto) run_on_active(const F &f) {
      using ret_t = decltype(f(get<0>()));
      ((if constexpr (is_active<T>()) return f(get<T>())),...);
    } */
};
} // namespace compile_time