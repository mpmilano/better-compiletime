#pragma once
#include "compile_time_context.hpp"
#include "instance.hpp"
#include "types.hpp"

namespace compile_time {
namespace type_to_value {

template <typename expected_return, typename A, typename B>
constexpr auto convert_to_value(A &a, const B &b);

template <typename Allocator, typename Field, typename field_type>
constexpr void fill_in_field(Allocator &a, Field &i, const field_type &field) {
  i = convert_to_value<DECT(i)>(a, field);
}

template <typename Allocator, typename Field, typename field_type>
constexpr void fill_in_field(Allocator &a, value::instance<Field> &i,
                             const field_type &field) {
  i = convert_to_value<DECT(i)>(a, field);
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
  ptr.set(ctctx::allocate<T>(a),
          a.template as_single_allocator<value::convert_to_instance_t<T>>());
  fill_in_field(a, ptr.template get<value::convert_to_instance_t<T>>(a), field);
}

template <typename Allocator>
constexpr void fill_in_field(Allocator &, value::top_pointer &ptr,
                             const types::null_type) {
  // PTR stays null.
  ptr.clear();
}

template <typename expected_return, typename Allocator, typename T,
          typename... fields>
constexpr auto _convert_to_value(Allocator &a,
                                 const types::instance<T, fields...> &) {
  value::convert_to_instance_t<T> ret;
  ret.match([&a](auto &... _fields) constexpr {
    ((fill_in_field(a, _fields, fields{})), ...);
  });
  return ret;
};

template <typename expected_return, typename Allocator, typename Val, Val val>
constexpr auto _convert_to_value(Allocator &,
                                 const types::raw_value<Val, val> &) {
  return val;
}

template <typename expected_return, typename Allocator, char... str>
constexpr auto _convert_to_value(Allocator &, const mutils::String<str...> &s) {
  value::string s2;
  mutils::cstring::str_cpy(s2.strbuf, s.string);
  return s2;
}

template <typename expected_return, typename Allocator, char... str>
constexpr auto _convert_to_value(Allocator &, const types::error<str...> &) {
  error_t s2;
  constexpr const char rstr[sizeof...(str) + 1] = {str..., 0};
  mutils::cstring::str_cpy(s2.msg, rstr);
  return s2;
}

template <typename> struct is_value_list : public std::false_type {};
template <typename T>
struct is_value_list<value::list<T>> : public std::true_type {};

template <typename expected_return, typename Allocator, typename... T>
struct list_helper {
  template <std::size_t... indices>
  static constexpr auto
  _convert_to_value(Allocator &a, const types::list<T...> &,
                    const std::integer_sequence<std::size_t, indices...> &) {
    static_assert(is_value_list<expected_return>::value,
                  "Error: list expected but not encountered.");
    expected_return l;
    (fill_in_field(a, l.values[indices], T{}), ...);
    l.size = sizeof...(T);
    return l;
  }
};

template <typename expected_return, typename Allocator, typename... T>
constexpr auto _convert_to_value(Allocator &a, const types::list<T...> &s) {
  return list_helper<expected_return, Allocator, T...>::_convert_to_value(
      a, s, std::make_index_sequence<sizeof...(T)>{});
}

template <typename expected_return, typename A, typename B>
constexpr auto convert_to_value(A &a, const B &b) {
  return _convert_to_value<expected_return>(a, b);
}
} // namespace type_to_value

template <typename Allocator, typename T>
constexpr Allocator convert_to_value() {
  Allocator a;
  auto &&ref = type_to_value::convert_to_value<DECT(a.top)>(a, T{});
  a.top.operator=(ref);
  return a;
}
} // namespace compile_time