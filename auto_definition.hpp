#pragma once
#include "specification.hpp"

namespace compile_time{
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
    namespace value {
    template<typename T> using convert_to_instance_t = typename specification::convert_to_instance<T>::type; 

            template<std::size_t N, std::size_t max_size, typename client> struct auto_defn_impl;
#define auto_defn_impl_body_23423 \
            static_assert(max_size > 0);\
            using argN_t = convert_to_instance_t<DECT(boost::pfr::get<N>(std::declval<client>()))>;\
            argN_t arg{};\
            constexpr argN_t& get(std::integral_constant<std::size_t, N> const * const) {\
                return arg;\
            }\
            constexpr const argN_t& get(std::integral_constant<std::size_t, N> const * const) const {\
                return arg;\
            }\
            constexpr auto_defn_impl() = default;

        template<std::size_t max_size, typename client> struct auto_defn_impl<0,max_size,client>{
            inline static constexpr std::size_t N = 0;
            auto_defn_impl_body_23423
        };

        template<std::size_t N, std::size_t max_size, typename client> struct auto_defn_impl : public auto_defn_impl<N-1,max_size,client> {
            using auto_defn_impl<N-1,max_size,client>::get;
            auto_defn_impl_body_23423
        };

        template<typename client> struct auto_definition : public auto_defn_impl<struct_size<client>-1, struct_size<client>, client > {
            static_assert(struct_size<client> >= 0);
            using parent = auto_defn_impl<struct_size<client>-1, struct_size<client>, client >;
            using parent::get;
            template<std::size_t N> constexpr const auto& get() const {
                constexpr mutils::num_as_ptr<N> num_as_type {nullptr};
                return get(num_as_type);
            }
            template<std::size_t N> constexpr auto& get() {
                constexpr mutils::num_as_ptr<N> num_as_type {nullptr};
                return get(num_as_type);
            }
        };
    }
}