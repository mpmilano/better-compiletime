#pragma once
#include <type_traits>
#include <boost/pfr/precise.hpp>

#define CONSTVARY(name, body...) name body name const body
#define CONSTVARY2(name, name2, body...) name, name2 body name, name2 const body

#define DECT(...) std::decay_t<decltype(__VA_ARGS__)>

namespace compile_time {

    template<typename F>
    struct wrap_invocation {
        static const constexpr DECT(F{}()) value{/*F{}()*/};
        constexpr wrap_invocation() {
            value = F{}();
        }
        constexpr auto& operator()() const {return value;}
    };

#define struct_wrap(name, invocation...) struct __ ## name { constexpr __ ## name() = default; constexpr decltype(auto) operator()() const {return invocation;}}; using name = wrap_invocation<__ ## name>;


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

    template<typename T>
    inline constexpr std::size_t struct_size = boost::pfr::tuple_size<T>::value;
}

namespace mutils{

template <class T, class Enable = void>
struct is_defined : std::false_type{};

template <class T>
struct is_defined<T, std::enable_if_t<(sizeof(T) > 0)> > : std::true_type{};
template<class T> inline constexpr bool is_defined_v = is_defined<T>::value;
template<std::size_t N> using num_as_ptr = std::integral_constant<std::size_t, N>*;
}