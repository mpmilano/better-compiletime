#include "provided_values.hpp"
#include "utils.hpp"
#include "specification.hpp"
#include "instance.hpp"
#include "types.hpp"
#include "convert.hpp"
#include "compile_time_context.hpp"
#include "mutils/cstring.hpp"
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
        statement body;
    };

    struct assign{
        varname var;
        expression value;
    };

    struct Return {
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

    template<std::size_t max_str_size>
    struct parser : public ctctx::value_info<program, statement, expression, sequence, skip, declare, assign, Return, constant<std::size_t>, binop<'+'>, varref> {
        private:
        using string_length = std::integral_constant<std::size_t,max_str_size>;
        using str_t = char const[string_length::value + 1];
        using str_nc = char[string_length::value + 1];
            template<std::size_t s>
            using fixed_cstr = mutils::cstring::fixed_cstr<s>;
            template<std::size_t s>
            using fixed_str = mutils::cstring::fixed_str<s>;

            template<std::size_t str_size>
            constexpr ct<expression> parse_expression(const fixed_cstr<str_size> &){
                return ct<expression>{};
            }

            template<std::size_t str_size>
            constexpr ct<statement> parse_return(const fixed_cstr<str_size> &str){
                using namespace mutils::cstring;
                ct<statement> ret;
                ret.match([&](value::void_pointer& stmt) constexpr {
                    auto dec = allocate<Return>();
                    str_nc ret_expr = {0};
                    remove_first_word(ret_expr, str);
                    deref(dec).match([&](auto& expr) constexpr {
                        expr = parse_expression(ret_expr);
                    });
                    stmt.set(std::move(dec),single_allocator<Return>());
                });
                return ret;
            }

            template<std::size_t str_size>
            constexpr ct<statement> parse_declare(const fixed_cstr<str_size> &str){
                using namespace mutils::cstring;
                ct<statement> ret;
                ret.match([&](value::void_pointer& stmt) constexpr {
                    auto dec = allocate<declare>();
                    str_nc let_expr = {0};
                    remove_first_word(let_expr, str);
                    str_nc let_components[2] = {{0}};
                    first_split(',', let_expr, let_components);
                    deref(dec).match([&](auto& var, auto& expr, auto& body) constexpr {
                        str_nc string_bufs[2] = {{0}};
                        split_outside_parens('=', let_components[0], string_bufs);
                        str_cpy(var.strbuf,string_bufs[0]);
                        expr = parse_expression(string_bufs[1]);
                        body = parse_statement(let_components[1]);
                    });
                    stmt.set(std::move(dec),single_allocator<declare>());
                });
                return ret;
            }
            template<std::size_t str_size>
            constexpr ct<statement> parse_assignment(const fixed_cstr<str_size> &in){
                using namespace mutils::cstring;
                ct<statement> ret;
                ret.match([&](value::void_pointer& stmt) constexpr {
                    auto dec = allocate<assign>();
                    deref(dec).match([&](auto& l, auto& r) constexpr {
                        str_nc string_bufs[2] = {{0}};
                        split_outside_parens('=', in, string_bufs);
                        str_cpy(l.strbuf,string_bufs[0]);
                        r = parse_expression(string_bufs[1]);
                    });
                    stmt.set(std::move(dec),single_allocator<assign>());
                });
                return ret;
            }
            template<std::size_t str_size>
            constexpr ct<statement> parse_sequence(const fixed_cstr<str_size> &str){
                using namespace mutils::cstring;
                str_nc string_bufs[2] = {{0}};
                first_split(',', str, string_bufs);
                ct<statement> ret;
                ret.match([&](value::void_pointer& stmt) constexpr {
                    auto dec = allocate<sequence>();
                    deref(dec).match([&](auto& fst, auto& rst) constexpr {
                        fst = parse_statement(string_bufs[0]);
                        rst = parse_statement(string_bufs[1]);
                    });
                    stmt.set(std::move(dec),single_allocator<sequence>());
                });
                return ret;
            }

            template<std::size_t str_size>
            constexpr ct<statement> parse_statement(const fixed_cstr<str_size> &in){
                using namespace mutils::cstring;
                if (first_word_is("var",in)){
                    return parse_declare(in);
                }
                else if (contains_outside_parens(',', in)){
                    return parse_sequence(in);
                }
                else if (contains_outside_parens("=", in)){
                    return parse_assignment(in);
                }
                else if (first_word_is("return",in)){
                    return parse_return(in);
                }
                else return ct<statement>{};
            }

public:
            constexpr ct<program>& parse_program(const fixed_cstr<max_str_size> &str){
                allocator.top.match([this,str](ct<statement>& body) constexpr {body = parse_statement(str);});
                return allocator.top;
            }
            const ct<program> parsed;
            constexpr parser(const fixed_cstr<max_str_size> &str)
            :parsed(parse_program(str)){}
    };

}
}

using namespace compile_time;
using namespace simple_language;

#define CONVERT_T(x...) compile_time_context<x>::template convert_to_type<x>
#define PARSE(x...) parser<mutils::cstring::str_len(#x) + 1>{#x}

constexpr decltype(auto) parse_trial(){
    return PARSE(return 3 + 5, return 2 + 6, return 0 + 1).allocator;
}

struct convert_parsed {
    static const constexpr DECT(parse_trial()) allocator{parse_trial()};
    static constexpr const DECT(allocator.top)& value = allocator.top;
    constexpr const auto& operator()() const { return value;}
    constexpr convert_parsed() = default;
};


using step1 = CONVERT_T(convert_parsed);

int main(){
    constexpr parser<28> parsed{"var x = 3 + 5, x + x, 7 + 8"};
    std::cout << &parsed.parsed << std::endl;
    step1::print();
}