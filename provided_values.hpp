#include "specification.hpp"

namespace compile_time {
    
    namespace value
    {
        template<typename T> struct list{
            constexpr list() = default;
        };
    } // value
    
    namespace specification {
        template<typename T> struct convert_to_instance<list<T> > {using type = value::list<T>;};
    }
}