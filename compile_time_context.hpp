#pragma once
#include "utils.hpp"

namespace compile_time {
    template<typename top, typename... specs>
    struct compile_time_context {

    template<typename Fvalue> static constexpr auto convert_to_type_f();
    template<typename Fvalue> using convert_to_type = DECT(convert_to_type_f<Fvalue>());

    };
}