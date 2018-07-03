#pragma once
#include "auto_specification.hpp"

namespace compile_time{
    namespace specification {
        template<typename T> struct permitted_raw : std::false_type{};
        template<> struct permitted_raw<int> : std::true_type{};

        template<typename T> struct pointer{};
        struct void_pointer{};
        template<typename T> struct list{};

        template<typename T> struct convert_to_instance;
    }
}