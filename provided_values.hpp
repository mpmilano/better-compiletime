#pragma once

#include "specification.hpp"
#include "allocated_ref.hpp"

namespace compile_time {
    
    namespace value
    {
        template<typename T> struct list{
            constexpr list() = default;
        };
        struct string{
            constexpr string() = default;
            char strbuf[1028] = {};
        };
        using void_pointer = erased_ref;
        template<typename T>
        using pointer = allocated_ref<T>;
    } // value
    
    namespace specification {
        template<typename T> struct convert_to_instance<list<T> > {using type = value::list<T>;};
        template<> struct convert_to_instance<string> {using type = value::string;};
        template<> struct convert_to_instance<void_pointer> {using type = value::void_pointer;};
        template<typename T> struct convert_to_instance<pointer<T> > {using type = value::pointer<T>;};
    }
}