#pragma once
#include "compile_time_context.hpp"
#include <tuple>
#include "specification.hpp"
#include "types.hpp"

namespace compile_time {
    namespace types{
        template<typename T, typename... fields>
        struct instance_build{
            constexpr instance_build() = default;
            template<typename... morefields>
            constexpr static auto append(instance_build<T, morefields...>) {
                return instance_build<T,fields...,morefields...>{};
            }
        };
    }
    namespace value_to_type {
        using namespace value;

        template<typename FValue, std::size_t field>
        struct get_field{
            constexpr static auto& value = FValue::value.template get<field>();
            constexpr auto& operator()() const {
                return value;
            }
        };

        template<typename FValue>
        struct simple_wrapper {
            static constexpr const DECT(FValue::value) &value = FValue::value;
            constexpr simple_wrapper() = default;
            constexpr auto& operator()() const {
                return value;
            }
        };

        template<typename T1, typename... T> constexpr types::instance<T1,T...>* instance_indirection(){
            return nullptr;
        }

        template<typename FValue, typename ctcx, typename T, std::size_t... indexes> constexpr auto convert_to_type_instance(std::integer_sequence<std::size_t,indexes...>){
            using Value = simple_wrapper<FValue>;
            struct_wrap(return_t, types::instance<T,DECT(ctcx::template convert_to_type_f<get_field<Value,indexes>>().value)...>{};);
            return simple_wrapper<return_t>{};
        }

        template<typename FValue, typename Allocator_holder, typename top, typename T, typename... specs> constexpr auto convert_to_type_f(instance<T> const * const, types::wrapped_type<specs>...){
            using ctctx = compile_time_context<Allocator_holder, top, specs...>;
            return convert_to_type_instance<FValue,ctctx,T>(std::make_index_sequence<struct_size<T>>{});
        }

        template<typename FValue, typename ctcx, typename spec1, typename... specs> 
        constexpr auto convert_to_type_erased_ref(std::enable_if_t<FValue{}()>* = nullptr){
            constexpr auto &ptr = FValue::value;
            if constexpr(ptr.template is_this_type<spec1>(ctcx::allocator)){
                struct_wrap(converted,FValue::value.get(ctcx::allocator.template as_single_allocator<spec1>()));
                using wrapped = simple_wrapper<converted>;
                return ctcx::template convert_to_type_f<wrapped>();
            }
            else return convert_to_type_erased_ref<FValue, ctcx, specs...>();
            //no base-case, because we should never run past the end of this list!
        }

        template<typename FValue, typename Allocator_holder, typename top, typename... specs> constexpr auto convert_to_type_f(erased_ref const * const, types::wrapped_type<specs>...){
            using ctcx = compile_time_context<Allocator_holder, top, specs...>;
            return convert_to_type_erased_ref<FValue,ctcx,specs...>();
        }

        template<typename FValue, typename Allocator_holder, typename top, typename T, typename... specs> constexpr auto convert_to_type_f(allocated_ref<T> const * const, types::wrapped_type<specs>...){
            using ctcx = compile_time_context<Allocator_holder, top, specs...>;
            struct_wrap(converted,FValue::value.get(ctcx::allocator.template as_single_allocator<T>()));
            using wrapped = simple_wrapper<converted>;
            return ctcx::template convert_to_type_f<wrapped>();
        }

        template<typename FValue, typename Allocator_holder, typename top, typename T, typename... specs> constexpr auto convert_to_type_f(list<T> const * const, types::wrapped_type<specs>...){
            //TODO: actually read the list contents. 
            struct return_t {
                types::list<> value{};
                constexpr return_t() = default;
            };
            return return_t{};
        }
    }
    namespace ctctx {
    template<typename Allocator_holder, typename top, typename... specs> template<typename FValue>
    constexpr auto i<Allocator_holder, top, specs...>::convert_to_type_f(){
        using namespace value_to_type;
        constexpr const auto &value = FValue::value;
        using value_t = DECT(value);
        if constexpr (specification::is_permitted_raw<value_t>){
            constexpr const auto _value = FValue::value;
            struct return_t {
                types::raw_value<value_t, _value> value{};
                constexpr return_t() = default;
            };
            return return_t{};
        }
        else return value_to_type::convert_to_type_f<FValue, Allocator_holder, top>(&value, types::wrapped_type<specs>{}...);
    }
    }
}