#pragma once
#include "compile_time_context.hpp"
#include "instance.hpp"
#include "types.hpp"

namespace compile_time {
namespace type_to_value {

template <typename A, typename B>
constexpr auto convert_to_value(A &a, const B &b);

template <typename Allocator, typename Field, typename field_type>
constexpr void fill_in_field(Allocator &a, Field &i, const field_type &field) {
  i = convert_to_value(a, field);
}

template <typename Allocator, typename Field, typename field_type>
constexpr void fill_in_field(Allocator &a, value::instance<Field> &i,
                             const field_type &field) {
  i = convert_to_value(a, field);
}

template <typename Allocator, typename Field, typename field_type>
constexpr void fill_in_field(Allocator &a, value::pointer<Field> &ptr,
                             const field_type &field) {
  ptr = a.template allocate<Field>();
  fill_in_field(a, ptr.get(a), field);
}

template <typename Allocator, typename T, typename... fields>
constexpr void fill_in_field(Allocator &a, value::top_pointer &ptr,
                             const types::instance<T, fields...> &field) {
  ptr.set(ctctx::allocate<T>(a), a);
  fill_in_field(a, ptr.template get<value::convert_to_instance_t<T>>(a), field);
}

template <typename Allocator>
constexpr void fill_in_field(Allocator &, value::top_pointer &ptr,
                             const types::null_type) {
  // PTR stays null.
  ptr.clear();
}

template <typename Allocator, typename T, typename... fields>
constexpr auto _convert_to_value(Allocator &a,
                                 const types::instance<T, fields...> &) {
  value::convert_to_instance_t<T> ret;
  ret.match([&a](auto &... _fields) constexpr {
    ((fill_in_field(a, _fields, fields{})), ...);
  });
  return ret;
};

template <typename Allocator, typename Val, Val val>
constexpr auto _convert_to_value(Allocator &,
                                 const types::raw_value<Val, val> &) {
  return val;
}

template <typename Allocator, char... str>
constexpr auto _convert_to_value(Allocator &, const mutils::String<str...> &s) {
  value::string s2;
  mutils::cstring::str_cpy(s2.strbuf, s.string);
  return s2;
}

template <typename A, typename B>
constexpr auto convert_to_value(A &a, const B &b) {
  return _convert_to_value(a, b);
}
} // namespace type_to_value

template <typename Allocator, typename T>
constexpr Allocator convert_to_value() {
  Allocator a;
  auto &&ref = type_to_value::convert_to_value(a, T{});
  a.top.operator=(ref);
  return a;
}
} // namespace compile_time