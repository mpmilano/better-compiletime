#pragma once

#include "allocated_ref.hpp"
#include "specification.hpp"

namespace compile_time {

namespace value {
template <typename T> struct list { constexpr list() = default; };
struct string {
  constexpr string() = default;
  char strbuf[1028] = {};
};
using top_pointer = erased_ref;
template <typename T> using pointer = allocated_ref<T>;
} // namespace value

namespace specification {
template <typename T> struct convert_to_instance<list<T>> {
  using type = value::list<T>;
};
template <> struct convert_to_instance<string> { using type = value::string; };
template <> struct convert_to_instance<top_pointer> {
  using type = value::top_pointer;
};
template <typename T> struct convert_to_instance<pointer<T>> {
  using type = value::pointer<T>;
};
} // namespace specification
} // namespace compile_time