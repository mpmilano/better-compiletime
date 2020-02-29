#pragma once
#include "auto_specification.hpp"
#include "specification.hpp"
#include "utils.hpp"

namespace compile_time {
namespace specification {
template <typename T> struct convert_to_type;
}
namespace types {

template <typename... T> struct list {
  constexpr list() = default;
  template <typename... U>
  static constexpr list<T..., U...> append(const list<U...> &) {
    return list<T..., U...>{};
  }
};
template <char...> struct string { constexpr string() = default; };

template <typename T, typename... Fields> struct instance {
  static_assert(sizeof...(Fields) == struct_size<T>,
                "Error: incorrect number of fields");
  constexpr instance() = default;
  static const constexpr bool all_ok{true};
  static const constexpr bool is_error{false};
};
struct null_type {
  constexpr null_type() = default;
};
// template<typename T> struct default_convert<false, list<T> >
// {template<typename... Fields> using type = instance<T, Fields...>;};
template <typename T, T> struct raw_value { constexpr raw_value() = default; };

template <char... str> struct error {
  static constexpr const char msg[sizeof...(str) + 1] = {str..., 0};
  static const constexpr bool is_error{true};
};

template <char... str> constexpr bool is_error(const error<str...> &) {
  return true;
}

template <typename T> constexpr bool is_error(T &&) { return false; }

template <char... str>
std::ostream &operator<<(std::ostream &o, const error<str...> &e) {
  return o << e.msg;
}

template <char... c>
constexpr auto error_from_ctstring(const mutils::String<c...> &) {
  return error<c...>{};
}

template <typename FV> constexpr auto error_from_value_error_f() {
  struct i {
    constexpr i() = default;
    constexpr const char *operator()() const { return FV{}().msg; }
  };
  return error_from_ctstring(mutils::cstring::build_type_string<i>());
}

template <typename T>
using error_from_value_error = DECT(error_from_value_error_f<T>());

template <typename T> struct wrapped_type {
  constexpr wrapped_type() = default;
};
} // namespace types

} // namespace compile_time
