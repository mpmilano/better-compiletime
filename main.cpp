
#include <boost/pfr/precise.hpp>
#include <iostream>
#include <type_traits>
#include "utils.hpp"
#include "specification.hpp"
#include "instance.hpp"


namespace compile_time {
    namespace client{
        struct D{
            int five;
        };
        struct C{
            int four;
        };
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
    //add things to the specification -- types to allow raw, or explicit overrides of how types should be converted to instances. 
    // Also user_definitions of converted classes. 
    namespace specification{
        template<> struct convert_to_instance<client::C> {using type = client::C;};
        template<> struct user_definition<client::D> : public client::D {};
    }
}

using namespace compile_time;


constexpr auto try_me(){
    value::convert_to_instance_t<client::B> b{};
    b.match([] (int& three) constexpr {three = 3;});
    return b;
}

constexpr auto try_harder(){
    value::convert_to_instance_t<client::A> a{};
    a.match([] (int& one, auto& two, value::instance<client::B>& three) constexpr {
        one = 1;
        three.match([](int& three) constexpr {three = 5;});
    });
    return a;
}

constexpr auto try_4(){
    value::convert_to_instance_t<client::D> d{};
    //D can match
    d.match([] (int& five) constexpr {
        five = 5;
    });
    //and also use its fields
    d.five++;
    return d;
}

int main(){

    constexpr value::instance<client::B> b = try_me();
    constexpr auto three = b.match([] (auto&& three) constexpr {return three;});
    static_assert(three == 3);
    constexpr auto a = try_harder();
    constexpr auto one = a.match([](const int& one, auto&&...){return one;});
    static_assert(one == 1);
    constexpr auto five = a.match([](auto&&,auto&&, const value::instance<client::B>& three) constexpr {
        return three.match([](const int& three) constexpr{return three;});
    });
    static_assert(five == 5);
    std::cout << three << std::endl;
    std::cout << one << std::endl;
    std::cout << five << std::endl;
}