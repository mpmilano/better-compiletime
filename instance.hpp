#pragma once
#include "auto_definition.hpp"
#include "matchbuilder.hpp"
#include "mutils/macro_utils.hpp"

namespace compile_time {
namespace value {

template <typename T> struct convert_to_instance;
template <typename T> struct permitted_raw;
} // namespace value
namespace specification {
template <typename client> struct user_definition;
}
namespace value {
template <typename client> struct auto_definition;
template <typename client>
using definition = std::conditional_t<
    mutils::is_defined_v<specification::user_definition<client>>,
    specification::user_definition<client>, auto_definition<client>>;

template <typename client> struct instance : public definition<client> {
  constexpr instance() = default;
  constexpr instance(instance &&) = default;
  constexpr instance(const instance &) = default;
  CONSTVARY(template <typename... args> constexpr auto match(args &&... a), {
    return matchBuilder<struct_size<client>, client>::match(
        *this, std::forward<args>(a)...);
  })
  using num_fields = std::integral_constant<std::size_t, struct_size<client>>;
  using field_index_sequence = std::make_index_sequence<num_fields::value>;
  using client_t = client;

  constexpr auto &operator=(const instance &o) {
    definition<client>::operator=(o);
    return *this;
  }
  constexpr auto &operator==(instance &&o) {
    definition<client>::operator=(o);
    return *this;
  }

private:
  // field offset translations
  template <std::size_t... indices>
  static constexpr std::size_t
  translate_field(std::size_t in, std::size_t offset_so_far,
                  std::integer_sequence<std::size_t, indices...> *ptr) {
    static_assert(sizeof...(indices) == num_fields::value,
                  "Internal error: call to translate_field");
    constexpr std::size_t field_sizes[num_fields::value] = {
        sizeof(boost::pfr::tuple_element_t<indices, client>)...};
    if (in == 0)
      return offset_so_far;
    else {
      return translate_field(in - field_sizes[offset_so_far], offset_so_far + 1,
                             ptr);
    }
  }

public:
  template <std::size_t in> constexpr decltype(auto) get_by_offset() {
    constexpr field_index_sequence *choice{nullptr};
    return this->template get<translate_field(in, 0, choice)>();
  }
  template <std::size_t in> constexpr decltype(auto) get_by_offset() const {
    constexpr field_index_sequence *choice{nullptr};
    return this->template get<translate_field(in, 0, choice)>();
  }

private:
  // the gets
  template <std::size_t N>
  constexpr decltype(auto) get(specification::user_definition<client> *) const {
    const definition<client> &_this = *this;
    return boost::pfr::get<N>(_this);
  }
  template <std::size_t N>
  constexpr decltype(auto) get(specification::user_definition<client> *) {
    definition<client> &_this = *this;
    return boost::pfr::get<N>(_this);
  }
  template <std::size_t N>
  constexpr decltype(auto) get(auto_definition<client> *) const {
    const definition<client> &_this = *this;
    return _this.template get<N>();
  }
  template <std::size_t N>
  constexpr decltype(auto) get(auto_definition<client> *) {
    definition<client> &_this = *this;
    return _this.template get<N>();
  }

public:
  template <std::size_t N> constexpr decltype(auto) get() const {
    constexpr definition<client> *swtch{nullptr};
    return get<N>(swtch);
  }
  template <std::size_t N> constexpr decltype(auto) get() {
    constexpr definition<client> *swtch{nullptr};
    return get<N>(swtch);
  }

  // utility

public:
  template <typename F> constexpr decltype(auto) self_apply(F &&f) {
    return f(*this);
  }

  template <typename F> constexpr decltype(auto) self_apply(F &&f) const {
    return f(*this);
  }
};

} // namespace value
} // namespace compile_time

#define FIELD(x...)                                                            \
  self_apply([](auto &&e) constexpr->decltype(auto) {                          \
    using type = typename DECT(e)::client_t;                                   \
    return e.template get_by_offset<offsetof(type, x)>();                      \
  })
