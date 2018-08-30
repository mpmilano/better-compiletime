#pragma once
#include "auto_definition.hpp"
#include "matchbuilder.hpp"

namespace compile_time {
    namespace value{

        template<typename T> struct convert_to_instance;
        template<typename T> struct permitted_raw;
    }
    namespace specification {
        template<typename client> struct user_definition;
    }
    namespace value {
        template<typename client> struct auto_definition;
        template<typename client> using definition 
        = std::conditional_t<mutils::is_defined_v<specification::user_definition<client> >, 
        specification::user_definition<client> , auto_definition<client> >;

        template<typename client> struct instance : public definition<client>{
            constexpr instance() = default;
            constexpr instance(instance&&) = default;
            constexpr instance(const instance&) = default;
            CONSTVARY(template<typename... args> constexpr auto match(args&&... a ),{
                return matchBuilder<struct_size<client>,client>::match(*this,std::forward<args>(a)...);
            })
            using num_fields = std::integral_constant<std::size_t, struct_size<client> >;
            constexpr auto& operator=(const instance& o){
                definition<client>::operator=(o);
                return *this;
            }
            constexpr auto& operator==(instance&& o){
                definition<client>::operator=(o);
                return *this;
            }
            private: 
            //the gets
            template<std::size_t N> constexpr decltype(auto) get(specification::user_definition<client>*) const {
                const definition<client>& _this = *this;
                return boost::pfr::get<N>(_this);
            }
            template<std::size_t N> constexpr decltype(auto) get(specification::user_definition<client>*)  {
                 definition<client>& _this = *this;
                return boost::pfr::get<N>(_this);
            }
            template<std::size_t N> constexpr decltype(auto) get(auto_definition<client>*) const {
                const definition<client>& _this = *this;
                return _this.template get<N>();
            }
            template<std::size_t N> constexpr decltype(auto) get(auto_definition<client>*)  {
                definition<client>& _this = *this;
                return _this.template get<N>();
            }
            public:
            template<std::size_t N> constexpr decltype(auto) get() const {
                constexpr definition<client>* swtch {nullptr};
                return get<N>(swtch);
            }
            template<std::size_t N> constexpr decltype(auto) get()  {
                constexpr definition<client>* swtch {nullptr};
                return get<N>(swtch);
            }
        };

    }
} // compile_time

