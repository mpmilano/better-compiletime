#pragma once
#include <cassert>
#include <cstddef>
#include <type_traits>

namespace compile_time {
template <std::size_t offset, typename... T> struct _tagged_union;

#define mutils_ctime_tu_common_accessors(L, indx, offset)                      \
  constexpr L &get(L const *const) { return data.indx; }                       \
  constexpr L &get(std::integral_constant<std::size_t, offset> const *const) { \
    return get((L *)nullptr);                                                  \
  }                                                                            \
  constexpr L &reset(L const *const) {                                         \
    if (active_member != active::_##L) {                                       \
      active_member = active::_##L;                                            \
      data = union_t{.indx = L{}};                                             \
    }                                                                          \
    return data.indx;                                                          \
  }

#define mutils_ctime_tu_common_A                                               \
  mutils_ctime_tu_common_accessors(A, fst, offset)
#define mutils_ctime_tu_common_B                                               \
  mutils_ctime_tu_common_accessors(B, snd, offset + 1)

#define mutils_ctime_tu_common_blank                                           \
  active active_member = active::None;                                         \
  union_t data = {.blank = uninit{}};

template <std::size_t offset, typename A> struct _tagged_union<offset, A> {
  constexpr _tagged_union() = default;
  struct uninit {};
  union union_t {
    A fst;
    uninit blank;
  };
  enum class active { _A, None };

  mutils_ctime_tu_common_blank mutils_ctime_tu_common_A
};

template <std::size_t offset, typename A, typename B>
struct _tagged_union<offset, A, B> {
  constexpr _tagged_union() = default;
  struct uninit {};
  union union_t {
    A fst;
    B snd;
    uninit blank;
  };
  enum class active { _A, _B, None };
  mutils_ctime_tu_common_blank mutils_ctime_tu_common_A mutils_ctime_tu_common_B
};

template <std::size_t offset, typename A, typename B, typename C,
          typename... rest>
struct _tagged_union<offset, A, B, C, rest...> {
  constexpr _tagged_union() = default;
  union union_t {
    A fst;
    B snd;
    _tagged_union<offset + 2, C, rest...> rst;
  };
  enum class active { _A, _B, Rst };
  active active_member = active::Rst;
  union_t data = {.rst = {_tagged_union<offset + 2, C, rest...>{}}};
  template <typename T> constexpr T &get(T const *const e) {
    return data.rst.get(e);
  }

  template <typename T> constexpr T &reset(T const *const e) {
    if (active_member != active::Rst) {
      active_member = active::Rst;
      data = union_t{.rst = _tagged_union<offset + 2, C, rest...>{}};
    }
    return data.rst.reset(e);
  }
  mutils_ctime_tu_common_A mutils_ctime_tu_common_B
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