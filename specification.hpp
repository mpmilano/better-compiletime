#pragma once
#include "auto_specification.hpp"

namespace compile_time {
namespace specification {
template <typename T> struct permitted_raw : std::false_type {};
// any arithmetic types will always be used raw.

template <typename T> struct pointer {};
struct top_pointer {};
template <typename T> struct list {};
struct string {};

template <typename T> struct convert_to_instance;
template <typename T> struct convert_to_type;
} // namespace specification
} // namespace compile_time