#pragma once
#include "utils.hpp"
#include "allocator.hpp"

namespace compile_time {
    constexpr inline std::size_t allocator_size = 25;
    namespace ctctx {
        template<typename top, typename... specs>
        using Allocator = compile_time::Allocator<allocator_size,value::convert_to_instance_t<top>,value::convert_to_instance_t<specs>...>;
        template<typename Allocator_holder, typename top_converted, typename... converted> struct i{
            static const constexpr compile_time::Allocator<allocator_size,top_converted, converted...> &allocator{Allocator_holder::allocator};
            template<typename Fvalue> static constexpr auto convert_to_type_f();
            template<typename Fvalue> using convert_to_type = typename DECT(convert_to_type_f<Fvalue>().value);
        };

        template<typename T, typename A>
        constexpr auto allocate(A && a){
            return a.template allocate<value::convert_to_instance_t<T>>();
        }
    }
    template<typename Allocator_holder, typename top, typename... specs> using compile_time_context = 
    ctctx::i<Allocator_holder, value::convert_to_instance_t<top>,value::convert_to_instance_t<specs>...>;
}