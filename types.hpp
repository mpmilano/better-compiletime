#pragma once
#include "auto_specification.hpp"
#include "specification.hpp"
#include "utils.hpp"

namespace compile_time
{
    namespace specification{
        template<typename T> struct convert_to_type;
    }
    namespace types{

        template<typename...> struct list{constexpr list() = default;};

        template<typename T, typename... Fields> struct instance {
            static_assert(sizeof...(Fields) == struct_size<T>, "Error: incorrect number of fields");
            constexpr instance() = default;
        };
        //template<typename T> struct default_convert<false, list<T> > {template<typename... Fields> using type = instance<T, Fields...>;};
        template<typename T, T> struct raw_value{constexpr raw_value() = default; };

        template<typename T> struct wrapped_type{ constexpr wrapped_type() = default; };
    }
    
} // compile_time

