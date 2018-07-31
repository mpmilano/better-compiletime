
#include <boost/pfr/precise.hpp>
#include <iostream>
#include <type_traits>
#include "utils.hpp"
#include "specification.hpp"
#include "instance.hpp"
#include "types.hpp"
#include "convert.hpp"


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
    a.match([] (int& one, auto& /*two*/, value::instance<client::B>& three) constexpr {
        one = 1;
        three.match([](int& three) constexpr {three = 5;});
    });
    return a;
}

constexpr auto try_3(){
    value::convert_to_instance_t<client::C> c{};
    //C can't match
    //c.match([](auto&&...) constexpr {});
    //but can ues its fields
    c.four = 4;
    return c;
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

struct boring_top{
        constexpr boring_top() = default;
        specification::void_pointer p{};
    };
struct boringer_body{
    constexpr boringer_body() = default;
    std::size_t i{0};
};

/* namespace compile_time::specification{
    template<> struct user_definition<boring_top> : public boring_top {constexpr user_definition() = default;};
    template<> struct user_definition<boringer_body> : public boringer_body {constexpr user_definition() = default;};
}*/

constexpr auto try_with_allocator_sub(){
    ctctx::Allocator<boring_top, boringer_body> a;
    auto ref = ctctx::allocate<boringer_body>(a);
    ref.get(a).match([](auto &i) constexpr {i = 5;});
    a.top.match([&](auto& p) constexpr {p = erased_ref{std::move(ref),a};});
    return a;
}

struct holder_for_try_with_allocator{
        static const constexpr ctctx::Allocator<boring_top, boringer_body> allocator{try_with_allocator_sub()};
    };



constexpr auto try_with_allocator(){
    using holder = holder_for_try_with_allocator;
    struct F { constexpr const DECT(holder::allocator.top)& operator()() const {
        return holder::allocator.top;
    } constexpr F() = default;
    };
    return compile_time_context<holder, boring_top, boringer_body>::template convert_to_type<F>{};
};

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
    constexpr auto c = try_3();
    constexpr auto four = c.four;
    constexpr auto d = try_4();
    constexpr auto six = d.five;
    static_assert(six == 6);
    std::cout << three << std::endl;
    std::cout << one << std::endl;
    std::cout << five << std::endl;
    std::cout << four << std::endl;
    std::cout << six << std::endl;
    struct_wrap(wrapped_harder, try_harder());
    using as_type = typename compile_time_context<client::A,client::B,client::C,client::D>::template convert_to_type<wrapped_harder>;
    //quash warning
    static_assert(!std::is_arithmetic_v<as_type>);

    constexpr const auto with_allocator = try_with_allocator();
}