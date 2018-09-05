#include "utils.hpp"
#include "specification.hpp"
#include "instance.hpp"
#include "types.hpp"
#include "convert.hpp"
#include "compile_time_context.hpp"
#include <iostream>

namespace compile_time {
namespace simple_language{
    using AST_node = specification::void_pointer;
    using varname = specification::string;

    struct statement{
        //i would like a more refined type here, ideally
        AST_node body;
    };

    struct expression{
        //i would like a more refined type here, ideally
        AST_node arg;
    };

    struct program{
        statement body;
    };

//statements

    struct sequence{
        statement fst;
        statement rst;
    };

    struct skip{};

    struct declare{
        varname var;
        expression value;
    };

    struct assign{
        varname var;
        expression value;
    };

//expressions

    template<typename Constant_t>
    struct constant{
        Constant_t value;
    };

    template<char oper>
    struct binop{
        expression left;
        expression right;
    };

    struct varref {
        varname var;
    };

    struct parser : public ctctx::value_info<program, statement, expression, sequence, skip, declare, assign, constant<std::size_t>, binop<'+'>, varref> {
        private:

            constexpr ct<statement> parse_statement(const char*){
                return ct<statement>{};
            }

public:
            constexpr ct<program>& parse_program(const char* str){
                allocator.top.match([this,str](ct<statement>& body) constexpr {body = parse_statement(str);});
                return allocator.top;
            }
            constexpr parser() = default;
    };

}
}

using namespace compile_time;
using namespace simple_language;

int main(){
    auto &prog = parser{}.parse_program("");
    std::cout << &prog << std::endl;
}