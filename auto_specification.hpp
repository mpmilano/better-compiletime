#pragma once

namespace compile_time{
    
    namespace specification
    {
        template<typename T> struct permitted_raw;
    } // specification
    
    namespace value{

    template<typename T> inline constexpr bool is_permitted_raw = specification::permitted_raw<T>::value;
    template<bool, typename> struct default_convert;
    template<typename T> struct default_convert<true,T> {using type = T;};
    template<typename client> struct instance;
    template<typename T> struct default_convert<false, T> {using type = value::instance<T>;};
    }
    namespace specification {
    template<typename T> struct convert_to_instance : public value::default_convert<value::is_permitted_raw<T>, T>{};
    }
}