#pragma once
#include "auto_specification.hpp"
#include "boost/pfr/precise.hpp"
#include "mutils/type_utils.hpp"
#include "provided_values.hpp"
#include "specification.hpp"
#include "utils.hpp"

namespace compile_time {
namespace value {

template <typename T>
using convert_to_instance_t =
    typename specification::convert_to_instance<T>::type;

template <std::size_t N, std::size_t max_size, typename client>
struct auto_defn_impl;
#define auto_defn_impl_body_23423_ctrs                                         \
  constexpr auto_defn_impl() = default;                                        \
  constexpr auto_defn_impl(const auto_defn_impl &) = default;                  \
  constexpr auto_defn_impl(auto_defn_impl &&) = default;                       \
  constexpr auto_defn_impl &operator=(auto_defn_impl &&) = default;

#define auto_defn_impl_body_23423                                              \
  static_assert(max_size > 0);                                                 \
  using argN_t =                                                               \
      convert_to_instance_t<DECT(boost::pfr::get<N>(std::declval<client>()))>; \
  argN_t arg{};                                                                \
  constexpr argN_t &get(std::integral_constant<std::size_t, N> const *const) { \
    return arg;                                                                \
  }                                                                            \
  constexpr const argN_t &get(                                                 \
      std::integral_constant<std::size_t, N> const *const) const {             \
    return arg;                                                                \
  }                                                                            \
  auto_defn_impl_body_23423_ctrs constexpr auto_defn_impl &operator=(          \
      const auto_defn_impl &o) {                                               \
    parent::operator=(o);                                                      \
    arg = o.arg;                                                               \
    return *this;                                                              \
  }

struct dummy_parent {};
template <std::size_t max_size, typename client>
struct auto_defn_impl<0, max_size, client> : public dummy_parent {
  using parent = dummy_parent;
  inline static constexpr std::size_t N = 0;
  auto_defn_impl_body_23423
};

template <std::size_t _N, typename client>
struct auto_defn_impl<_N, 0, client> {
  auto_defn_impl_body_23423_ctrs constexpr auto_defn_impl &
  operator=(const auto_defn_impl &o) = default;
  void get() {}
};

template <std::size_t N, std::size_t max_size, typename client>
struct auto_defn_impl : public auto_defn_impl<N - 1, max_size, client> {
  using parent = auto_defn_impl<N - 1, max_size, client>;
  using auto_defn_impl<N - 1, max_size, client>::get;
  auto_defn_impl_body_23423
};

template <typename client>
struct auto_definition : public auto_defn_impl<struct_size<client> - 1,
                                               struct_size<client>, client> {
  static_assert(struct_size<client> >= 0);
  using parent =
      auto_defn_impl<struct_size<client> - 1, struct_size<client>, client>;
  using parent::get;
  template <std::size_t N> constexpr const auto &get() const {
    constexpr mutils::num_as_ptr<N> num_as_type{nullptr};
    return get(num_as_type);
  }
  template <std::size_t N> constexpr auto &get() {
    constexpr mutils::num_as_ptr<N> num_as_type{nullptr};
    return get(num_as_type);
  }
  constexpr auto_definition() = default;
  constexpr auto_definition(const auto_definition &) = default;
  constexpr auto_definition(auto_definition &&) = default;
  constexpr auto &operator=(const auto_definition &o) {
    const parent &other = o;
    parent::operator=(other);
    return *this;
  }
};
} // namespace value
} // namespace compile_time