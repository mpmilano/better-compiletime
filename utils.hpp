#pragma once
#include <type_traits>

namespace compile_time {

#define DECT(...) std::decay_t<decltype(__VA_ARGS__)>

    template<typename T, typename fst, typename... rst>
    constexpr std::size_t index_of_f(){
        if constexpr (std::is_same_v<T,fst>) return 0;
        else return 1 + index_of_f<T,rst...>();
    }

    struct do_not_call_this{};
    template<std::size_t i, typename fst, typename... rst>
    fst type_at_f(std::enable_if_t<i == 0>* = nullptr){throw do_not_call_this{};}
    template<std::size_t i, typename fst, typename... rst>
    auto type_at_f(std::enable_if_t<(i > 0)>* = nullptr) -> DECT(type_at_f<i+1,rst...>()){throw do_not_call_this{};}

    template<typename T, typename... in> 
    inline constexpr std::size_t index_of = index_of_f<T,in...>();
    template<std::size_t i, typename... in>
    using type_at = DECT(type_at_f<i,in...>());
}