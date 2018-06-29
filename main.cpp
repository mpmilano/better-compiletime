
#include <boost/pfr/precise.hpp>
#define DECT(...) std::decay_t<decltype(__VA_ARGS__)>
#define CONSTVARY(name, body...) name body name const body
#define CONSTVARY2(name, name2, body...) name, name2 body name, name2 const body


namespace compile_time {

    template<typename T>
    inline constexpr std::size_t struct_size = boost::pfr::tuple_size<T>::value;

    namespace specification {
        template<typename T> struct pointer{};
        template<typename T> struct list{};
    }

    struct allocator{

    };

    struct value_list{

    };

    struct convertible_class {
    
    };

    namespace value {
        template<std::size_t N, typename client> struct instanceBuilder;
        template<typename client> struct instance;
        template<typename client> struct instanceBuilder<1,client>{
            using arg0_t = DECT(boost::pfr::get<0>(std::declval<client>()));
            arg0_t arg0;
            CONSTVARY2(template<typename F, typename... args> constexpr auto match(const instance<client>& c, F&& f, args&&... a ), {
                return f(arg0,std::forward<args>(a)...);
            })
            constexpr instanceBuilder() = default;
        };


        template<std::size_t N, typename client> struct instanceBuilder : public instanceBuilder<N-1,client>{
            using argN_t = DECT(boost::pfr::get<0>(std::declval<client>()));
            argN_t argN;
            CONSTVARY2(template<typename F, typename... args> constexpr auto match(const instance<client>& c, F&& f, args&&... a ), {
                return instanceBuilder<N-1,client>::match(c, [] (F f, argN_t an, args... a) constexpr {return f(an,std::forward<args>(a)...); }, std::forward<F>(f), argN, std::forward<args>(a)...);
            })
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

int main(){

    using namespace compile_time;
    value::instance<client::B> b;
    b.match([] (auto&& three) constexpr {return three;});
}