#pragma once

namespace compile_time {
namespace value {
template <std::size_t N, typename client> struct matchBuilder;
template <typename client> struct instance;

template <typename client> struct matchBuilder<0, client> {
  template <typename F, typename... args>
  static constexpr inline DO_INLINE decltype(auto) match(const instance<client> &, F &&f,
                                        args &&... a) {
    return f(std::forward<args>(a)...);
  }
  constexpr matchBuilder() = delete;
};

template <std::size_t N, typename client> struct matchBuilder {

  template <typename F, typename instance, typename... args>
  static constexpr inline DO_INLINE decltype(auto) match(instance &&c, F &&f, args &&... a) {
    return matchBuilder<N - 1, client>::match(
        std::forward<instance>(c), std::forward<F>(f), c.template get<N - 1>(),
        std::forward<args>(a)...);
  }
  constexpr matchBuilder() = delete;
};
} // namespace value
} // namespace compile_time
