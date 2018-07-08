#pragma once
#include "utils.hpp"
#include "allocator.hpp"

namespace compile_time {
    constexpr inline std::size_t allocator_size = 25;
    template<typename Allocator_holder, typename top, typename... specs>
    struct compile_time_context {
    static const constexpr Allocator<allocator_size,top,specs...> &allocator{Allocator_holder::allocator};
    constexpr compile_time_context(DECT(allocator)):allocator(std::move(allocator)){}
    template<typename Fvalue> static constexpr auto convert_to_type_f();
    template<typename Fvalue> using convert_to_type = DECT(convert_to_type_f<Fvalue>());

    };
}