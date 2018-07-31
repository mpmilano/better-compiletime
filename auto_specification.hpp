#pragma once

namespace compile_time{
    
    namespace specification
    {
        template<typename T> struct permitted_raw;
        template<typename T> inline constexpr bool is_permitted_raw = specification::permitted_raw<T>::value || std::is_arithmetic<T>::value;
    } // specification
    
    namespace value{
    template<bool, typename> struct default_convert;
    template<typename T> struct default_convert<true,T> {using type = T;};
    template<typename client> struct instance;
    template<typename T> struct default_convert<false, T> {using type = instance<T>;};
    }
    namespace types{
    template<typename, typename...> struct instance;
    template<bool, typename> struct default_convert;
    template<typename T> struct default_convert<true,T> {using type = T;};
    template<typename T> struct default_convert<false, T> {template<typename... Fields> using type = instance<T, Fields...>;};
    }
    namespace specification {
    template<typename T> struct convert_to_instance : public value::default_convert<is_permitted_raw<T>, T>{};
    template<typename T> struct convert_to_type : public types::default_convert<is_permitted_raw<T>, T>{};
    template<typename T> struct convert_to_instance<value::instance<T> > : public convert_to_instance<T> {};
    template<typename T> struct convert_to_type<types::instance<T> > : public convert_to_type<T> {};
    }
}