
#include <boost/pfr/precise.hpp>
#include <iostream>
#include <type_traits>
#define CONSTVARY(name, body...) name body name const body
#define CONSTVARY2(name, name2, body...) name, name2 body name, name2 const body


namespace compile_time {

    template<typename T>
    inline constexpr std::size_t struct_size = boost::pfr::tuple_size<T>::value;

    namespace specification {
        template<typename T> struct pointer{};
        struct void_pointer{};
        template<typename T> struct list{};
    }

    struct allocator{

    };

    struct value_list{

    };

    struct convertible_class {
    
    };

    template<typename compare, typename target>
    using const_if_const = std::conditional_t<std::is_const_v<compare>, const target, target>;

    namespace value {
        template<std::size_t N, typename client> struct instanceBuilder;
        template<typename client> struct instance;

        template<typename T> struct convert_to_instance_str{using type = T;};
        template<typename T> using convert_to_instance = typename convert_to_instance_str<T>::type;

        template<typename client> struct instanceBuilder<0,client>{
            CONSTVARY2(template<typename F, typename... args> constexpr auto match(const instance<client>& c, F&& f, args&&... a ), {
                return f(std::forward<args>(a)...);
            })
            constexpr instanceBuilder() = default;
        };

        

        template<std::size_t N, typename client> struct instanceBuilder : public instanceBuilder<N-1,client>{
            using argN_t = DECT(boost::pfr::get<0>(std::declval<client>()));
            convert_to_instance<argN_t> argN{};
            #define MATCHDEF_2394857(argt, decorator...) \
            template<typename F, typename... args> constexpr auto match(const instance<client>& c, F&& f, args&&... a ) decorator {\
                return instanceBuilder<N-1,client>::match(c, [] (F f, argt an, args... a) constexpr {return f(an,std::forward<args>(a)...); }, std::forward<F>(f), argN, std::forward<args>(a)...); \
            }
            MATCHDEF_2394857(argN_t&)
            MATCHDEF_2394857(const argN_t&, const)
            constexpr instanceBuilder() = default;
        };


        template<typename client> struct instance : public instanceBuilder<struct_size<client>,client>{
            constexpr instance() = default;
            CONSTVARY(template<typename... args> constexpr auto match(args&&... a ),{
                return instanceBuilder<struct_size<client>,client>::match(*this,std::forward<args>(a)...);
            })
        };
    }
}

namespace compile_time {
    namespace client{
        struct B{
            int three;
        };
        struct A{
            int one;
            specification::list<int> two;
            B three;
        };

        static_assert(boost::pfr::tuple_size<A>::value == 3, "sanity check pfr library works");
    }
}

using namespace compile_time;


constexpr auto try_me(){
    value::instance<client::B> b{};
    b.match([] (auto& three) constexpr {three = 3;});
    return b;
}

int main(){

    constexpr value::instance<client::B> b = try_me();
     auto three = b.match([] (auto&& three) constexpr {return three;});
    //static_assert(three == 0);
    std::cout << three << std::endl;
}