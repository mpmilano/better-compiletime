#pragma once
#include <cassert>
#include <cstddef>
namespace compile_time {
template <std::size_t size, typename T, typename Allocator>
struct SingleAllocator;
template <std::size_t s, typename Top, typename... Subs> struct Allocator;
struct erased_ref;

template <typename T> struct allocated_ref {
private:
  std::size_t indx;

  template <std::size_t size, typename Allocator>
  constexpr decltype(auto) construct(SingleAllocator<size, T, Allocator> &new_parent) {
    return new_parent.parent.slab[indx - 1].template reset<T>();
  }

public:
  template <std::size_t size, typename Allocator>
  constexpr T &get(SingleAllocator<size, T, Allocator> &new_parent) {
    return new_parent.parent.slab[indx - 1].template get<T>();
  }

  template <std::size_t size, typename Allocator>
  constexpr const T &
  get(SingleAllocator<size, T, Allocator> &new_parent) const {
    return new_parent.parent.slab[indx - 1].template get<T>();
  }

  template <std::size_t size, typename Allocator>
  constexpr const T &
  get(const SingleAllocator<size, T, Allocator> &new_parent) const {
    assert(indx > 0);
    return new_parent.parent.slab[indx - 1].template get<T>();
  }

  constexpr allocated_ref(const allocated_ref &) = delete;
  constexpr allocated_ref(allocated_ref &&o) : indx(o.indx) {}
  struct really_set_index {
    constexpr really_set_index() = default;
  };
  constexpr allocated_ref(const really_set_index &, std::size_t index)
      : indx{index} {}
  constexpr std::size_t get_index() const { return indx; }

  constexpr allocated_ref() : indx{0} {}

  template <std::size_t s, typename Allocator>
  constexpr allocated_ref(SingleAllocator<s, T, Allocator> &);

  template <std::size_t s, typename Allocator>
  constexpr void free(SingleAllocator<s, T, Allocator> &);

  constexpr allocated_ref &operator=(allocated_ref &&o) {
    indx = o.indx;
    return *this;
  }

  constexpr operator bool() const { return indx > 0; }
  friend struct erased_ref;
};

struct erased_ref {
private:
  std::size_t indx;
  std::size_t allocator_index;

public:
  template <typename T, std::size_t size, typename Allocator>
  constexpr T &get(SingleAllocator<size, T, Allocator> &new_parent) {
    return new_parent.parent.slab[indx - 1].template get<T>();
  }

  template <typename T, std::size_t size, typename Allocator>
  constexpr const T &
  get(SingleAllocator<size, T, Allocator> &new_parent) const {
    return new_parent.parent.slab[indx - 1].template get<T>();
  }

  template <typename T, std::size_t size, typename Allocator>
  constexpr const T &
  get(const SingleAllocator<size, T, Allocator> &new_parent) const {
    assert(indx > 0);
    return new_parent.parent.slab[indx - 1].template get<T>();
  }

  template <typename T, std::size_t size, typename Top, typename... Subs>
  constexpr bool
  is_this_type(const SingleAllocator<size, T, Allocator<size, Top, Subs...>> &,
               const Allocator<size, Top, Subs...> &a) const {
    return a.template get_allocator_index<
               SingleAllocator<size, T, Allocator<size, Top, Subs...>>>() ==
           allocator_index;
  }

  template <typename T, std::size_t size, typename Top, typename... Subs>
  constexpr bool is_this_type(const Allocator<size, Top, Subs...> &a) const {
    return is_this_type<T>(a, a);
  }

  constexpr erased_ref(const erased_ref &) = default;
  constexpr erased_ref(erased_ref &&o) = default;
  struct really_set_index {
    constexpr really_set_index() = default;
  };
  constexpr erased_ref(const really_set_index &, std::size_t index,
                       std::size_t allocator_index)
      : indx{index}, allocator_index{allocator_index} {}
  template <typename T, std::size_t n, typename Allocator>
  constexpr erased_ref(allocated_ref<T> &&o,
                       const SingleAllocator<n, T, Allocator> &a)
      : indx(o.indx), allocator_index(a.allocator_index) {}
  constexpr std::size_t get_index() const { return indx; }

  template <typename T, std::size_t n, typename Allocator>
  constexpr void set(allocated_ref<T> &&o,
                     const SingleAllocator<n, T, Allocator> &a) {
    indx = o.indx;
    allocator_index = a.allocator_index;
  }

  constexpr void clear() {
    indx = 0;
    allocator_index = 0;
  }

  constexpr erased_ref() : indx{0}, allocator_index{0} {}

  template <typename T, std::size_t s, typename Allocator>
  constexpr erased_ref(SingleAllocator<s, T, Allocator> &);

  template <typename T, std::size_t s, typename Allocator>
  constexpr void free(SingleAllocator<s, T, Allocator> &);

  constexpr erased_ref &operator=(erased_ref &&) = default;
  constexpr erased_ref &operator=(const erased_ref &) = default;

  constexpr operator bool() const { return indx > 0; }
};

} // namespace compile_time