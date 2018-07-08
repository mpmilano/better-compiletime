#include "specification.hpp"

namespace compile_time {
    
    namespace value
    {
        template<typename T> struct list{
            constexpr list() = default;
        };
        using void_pointer = erased_ref;
    } // value
    
    namespace specification {
        template<typename T> struct convert_to_instance<list<T> > {using type = value::list<T>;};
        template<typename T> struct convert_to_instance<void_pointer> {using type = value::void_pointer;};
    }
}