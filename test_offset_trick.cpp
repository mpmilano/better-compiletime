#include <cstddef>
#include <type_traits>
#include <boost/pfr/precise.hpp>
#include <cassert>

struct A{
    int i; double b;
};

struct test {
    A a;
    int b;
    unsigned long long d;
};

template<typename test, std::size_t... indices>
struct converted_test {
    using num_fields = std::integral_constant<std::size_t,boost::pfr::tuple_size<test>::value>;
    static_assert(sizeof...(indices) == num_fields::value);
    static constexpr std::size_t field_sizes[num_fields::value] = {sizeof(boost::pfr::tuple_element_t<indices,test>)...};
    static constexpr std::size_t translate_field(std::size_t in, std::size_t offset_so_far = 0){
        if (in == 0) return offset_so_far;
        else {
            return translate_field(in - field_sizes[offset_so_far], offset_so_far + 1);
        }
    }
};

#define DECT(x...) std::decay_t<decltype(x)>
#define field(a,x) boost::pfr::get<converted_test<DECT(a),0,1,2>::translate_field(offsetof(DECT(a),x))>(a)

int main(){
    converted_test<test,0,1,2> t;
    test t2;
    t2.b = 4;
    assert(t2.b == field(t2,b));
    return field(t2,b);
}