#pragma once
#include "allocator.hpp"
#include "auto_definition.hpp"
#include "utils.hpp"

namespace compile_time {
constexpr inline std::size_t allocator_size = 25;
namespace ctctx {
template <typename top, typename... specs>
using Allocator =
    compile_time::Allocator<allocator_size, value::convert_to_instance_t<top>,
                            value::convert_to_instance_t<specs>...>;
template <typename Allocator_holder> struct i {
  using Allocator = DECT(Allocator_holder::allocator);
  static const constexpr Allocator &allocator{Allocator_holder::allocator};
  template <typename Fvalue> static constexpr auto convert_to_type_f();
  template <typename Fvalue>
  using convert_to_type = typename DECT(convert_to_type_f<Fvalue>().value);
};

template <typename T, typename A> constexpr decltype(auto) allocate(A &&a) {
  return a.template allocate<value::convert_to_instance_t<T>>();
}

template <typename top, typename... specs> struct compile_time_workspace {
  using Allocator = ::compile_time::ctctx::Allocator<top, specs...>;
  Allocator allocator{};
  template <typename T> using ct = value::convert_to_instance_t<T>;
  constexpr compile_time_workspace() = default;
  constexpr compile_time_workspace(const compile_time_workspace &) = default;
  constexpr compile_time_workspace(compile_time_workspace &&) = default;
  constexpr compile_time_workspace &
  operator=(const compile_time_workspace &) = default;
  constexpr compile_time_workspace &
  operator=(compile_time_workspace &&) = default;

  template <typename T> constexpr decltype(auto) allocate() {
    static_assert(((std::is_same_v<specs, T>)+... + 0) == 1,
                  "Error: attempt to allocate unregistered type");
    return ctctx::allocate<T>(allocator);
  }

  template <typename T> constexpr decltype(auto) single_allocator() {
    static_assert(((std::is_same_v<specs, T>)+... + 0) == 1,
                  "Error: attempt to use unregistered type");
    return allocator.template as_single_allocator<ct<T>>();
  }

  template <typename T> constexpr decltype(auto) top_allocate() {
    static_assert(((std::is_same_v<specs, T>)+... + 0) == 1,
                  "Error: attempt to use unregistered type");
    value::top_pointer ret;
    ret.set(allocate<T>(), single_allocator<T>());
    return ret;
  }

  template <typename T>
  constexpr decltype(auto) deref(value::pointer<value::instance<T>> &p) {
    static_assert(((std::is_same_v<specs, T>)+... + 0) == 1,
                  "Error: attempt to use unregistered type");
    return p.get(allocator);
  }

  template <typename T>
  constexpr decltype(auto) set_pointer(value::top_pointer e,
                                       value::pointer<value::instance<T>> &&p) {
    static_assert(((std::is_same_v<specs, T>)+... + 0) == 1,
                  "Error: attempt to use unregistered type");
    return e.set(std::move(p), single_allocator<T>());
  }

  template <typename T>
  constexpr decltype(auto) upcast(value::pointer<value::instance<T>> &&p) {
    return value::top_pointer{std::move(p), single_allocator<T>()};
  }
};
} // namespace ctctx
template <typename Allocator_holder>
using compile_time_context = ctctx::i<Allocator_holder>;
} // namespace compile_time