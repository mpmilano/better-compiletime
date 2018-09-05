#pragma once
#include "utils.hpp"
#include "allocator.hpp"

namespace compile_time {
    constexpr inline std::size_t allocator_size = 25;
    namespace ctctx {
        template<typename top, typename... specs> using Allocator = 
        compile_time::Allocator<allocator_size,value::convert_to_instance_t<top>,value::convert_to_instance_t<specs>...>;
        template<typename Allocator_holder> struct i{
            using Allocator = DECT(Allocator_holder::allocator);
            static const constexpr Allocator &allocator{Allocator_holder::allocator};
            template<typename Fvalue> static constexpr auto convert_to_type_f();
            template<typename Fvalue> using convert_to_type = typename DECT(convert_to_type_f<Fvalue>().value);
        };

        template<typename T, typename A>
        constexpr decltype(auto) allocate(A && a){
            return a.template allocate<value::convert_to_instance_t<T>>();
        }

        template<typename top, typename... specs>
        struct value_info {
            using Allocator = ::compile_time::ctctx::Allocator<top,specs...>;
            Allocator allocator{};
            template<typename T> using ct = value::convert_to_instance_t<T>;
            constexpr value_info() = default;
            constexpr value_info(const value_info&) = default;
            constexpr value_info(value_info&&) = default;
            constexpr value_info& operator=(const value_info&) = default;
            constexpr value_info& operator=(value_info&&) = default;

            template<typename T> decltype(auto) allocate(){
                return ctctx::allocate<T>(allocator);
            }
        };
    }
    template<typename Allocator_holder> using compile_time_context = 
    ctctx::i<Allocator_holder>;
}