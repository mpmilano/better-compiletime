#pragma once
#include "compile_time_context.hpp"
#include "mutils/CTString.hpp"
#include "mutils/cstring.hpp"
#include "specification.hpp"
#include "types.hpp"
#include <tuple>

namespace compile_time {
namespace types {
template <typename T, typename... fields> struct instance_build {
  constexpr instance_build() = default;
  template <typename... morefields>
  constexpr static auto append(instance_build<T, morefields...>) {
    return instance_build<T, fields..., morefields...>{};
  }
};
} // namespace types
namespace value_to_type {
using namespace value;

template <typename FValue, std::size_t field> struct get_field {
  constexpr static auto &value = FValue::value.template get<field>();
  constexpr auto &operator()() const { return value; }
};

template <typename FValue> struct simple_wrapper {
  static constexpr const DECT(FValue::value) &value = FValue::value;
  constexpr simple_wrapper() = default;
  constexpr auto &operator()() const { return value; }
};

template <typename T1, typename... T>
constexpr types::instance<T1, T...> *instance_indirection() {
  return nullptr;
}

template <typename FValue, typename ctcx, typename T, std::size_t... indexes>
constexpr auto
convert_to_type_instance(const typename ctcx::Allocator &debug_allocator,
                         std::integer_sequence<std::size_t, indexes...>) {
  using Value = simple_wrapper<FValue>;
  // DEBUG
  (void)debug_allocator;
  ((ctcx::template convert_to_type_f<get_field<Value, indexes>>()), ...);
  struct_wrap(
      return_t,
      types::instance<
          T, DECT(ctcx::template convert_to_type_f<get_field<Value, indexes>>()
                      .value)...>{};);
  return simple_wrapper<return_t>{};
}

template <typename FValue, typename Allocator_holder, typename T>
constexpr auto convert_to_type_f(
    const typename compile_time_context<Allocator_holder>::Allocator
        &debug_allocator,
    instance<T> const *const inst) {
  (void)inst;
  using ctctx = compile_time_context<Allocator_holder>;
  return convert_to_type_instance<FValue, ctctx, T>(
      debug_allocator, std::make_index_sequence<struct_size<T>>{});
}

template <typename FValue, typename ctcx, std::size_t size, typename top,
          typename spec1, typename... specs>
constexpr auto convert_to_type_erased_ref(
    const typename ctcx::Allocator &debug_allocator,
    compile_time::Allocator<size, top, spec1, specs...> * = nullptr) {
  if constexpr (FValue{}()) {
    constexpr auto &ptr = FValue::value;
    if constexpr (ptr.template is_this_type<spec1>(ctcx::allocator)) {
      auto &as_single_allocator =
          ctcx::allocator.template as_single_allocator<spec1>();
      auto &value = FValue::value;
      auto &gotten = value.get(as_single_allocator);
      (void)gotten;
      (void)value;
      (void)as_single_allocator;
      struct_wrap(converted,
                  FValue::value.get(
                      ctcx::allocator.template as_single_allocator<spec1>()));
      auto converted_value = converted{}();
      (void)converted_value;
      using wrapped = simple_wrapper<converted>;
      auto wrapped_value = wrapped::value;
      (void)wrapped_value;
      return ctcx::template convert_to_type_f<wrapped>();
    } else
      return convert_to_type_erased_ref<FValue, ctcx, size, top, specs...>(
          debug_allocator);
    // no base-case, because we should never run past the end of this list!
  } else {
    struct_wrap(ret, types::null_type{});
    return simple_wrapper<ret>{};
  }
}

template <typename FValue, typename Allocator_holder>
constexpr auto convert_to_type_f(
    const typename compile_time_context<Allocator_holder>::Allocator
        &debug_allocator,
    erased_ref const *const) {
  using ctcx = compile_time_context<Allocator_holder>;
  return convert_to_type_erased_ref<FValue, ctcx>(
      debug_allocator, (typename ctcx::Allocator *)nullptr);
}

template <typename FValue, typename Allocator_holder, typename T>
constexpr auto convert_to_type_f(
    const typename compile_time_context<Allocator_holder>::Allocator
        &debug_allocator,
    allocated_ref<T> const *const) {
  using ctcx = compile_time_context<Allocator_holder>;
  struct_wrap(
      converted,
      FValue::value.get(ctcx::allocator.template as_single_allocator<T>()));
  using wrapped = simple_wrapper<converted>;
  return ctcx::template convert_to_type_f<wrapped>(debug_allocator);
}

template <typename FValue, typename Allocator_holder>
constexpr auto convert_to_type_f(
    const typename compile_time_context<Allocator_holder>::Allocator
        &debug_allocator,
    string const *const) {
  struct val {
    constexpr val() = default;
    constexpr const char *operator()() const { return FValue{}.value.strbuf; }
  };
  struct_wrap(ret, mutils::cstring::build_type_string<val>());
  return simple_wrapper<ret>{};
  (void)debug_allocator;
}

template <typename FValue, typename Allocator_holder, typename T>
constexpr auto convert_to_type_f(
    const typename compile_time_context<Allocator_holder>::Allocator
        &debug_allocator,
    list<T> const *const) {
  // TODO: actually read the list contents.
  (void)debug_allocator;
  struct return_t {
    types::list<> value{};
    constexpr return_t() = default;
  };
  return return_t{};
}

template <typename FValue, typename Allocator_holder, typename T>
constexpr auto convert_to_type_f(
    const typename compile_time_context<Allocator_holder>::Allocator
        &debug_allocator,
    maybe_error<T> const *const) {
  if constexpr (!FValue{}.value.error_set) {
    struct_wrap(ret, FValue{}.value.value);
    return convert_to_type_f<simple_wrapper<ret>, Allocator_holder>(
        debug_allocator, &FValue{}.value.value);
  } else {
    struct_wrap(err, FValue{}.value.error);
    struct return_t {
      types::error_from_value_error<err> value{};
      constexpr return_t() = default;
    };
    return return_t{};
  }
}

} // namespace value_to_type
namespace ctctx {
template <typename Allocator_holder>
template <typename FValue>
constexpr auto i<Allocator_holder>::convert_to_type_f() {
  // debug
  auto &allocator_ref = allocator;
  using namespace value_to_type;
  constexpr const auto &value = FValue::value;
  using value_t = DECT(value);
  if constexpr (specification::is_permitted_raw<value_t>) {
    constexpr const auto _value = FValue::value;
    struct return_t {
      types::raw_value<value_t, _value> value{};
      constexpr return_t() = default;
    };
    return return_t{};
  } else
    return value_to_type::convert_to_type_f<FValue, Allocator_holder>(
        allocator_ref, &value);
}
} // namespace ctctx
} // namespace compile_time