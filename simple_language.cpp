#include "compile_time_context.hpp"
#include "convert.hpp"
#include "instance.hpp"
#include "mutils/cstring.hpp"
#include "provided_values.hpp"
#include "specification.hpp"
#include "types.hpp"
#include "utils.hpp"
#include <iostream>

namespace compile_time {
namespace simple_language {
using AST_node = specification::top_pointer;
using varname = specification::string;

struct statement {
  // i would like a more refined type here, ideally
  AST_node body;
};

struct expression {
  // i would like a more refined type here, ideally
  AST_node arg;
};

struct program {
  statement body;
};

// statements

struct sequence {
  statement fst;
  statement rst;
};

struct declare {
  varname var;
  expression value;
  statement body;
};

struct assign {
  varname var;
  expression value;
};

struct Return {
  expression value;
};

struct list_test {
  specification::list<std::size_t> values;
  statement payload;
};

// expressions

template <typename Constant_t> struct constant { Constant_t value; };

template <char oper> struct binop {
  expression left;
  expression right;
};

struct varref {
  varname var;
};

template <std::size_t max_str_size>
struct parser
    : public ctctx::compile_time_workspace<
          program, statement, expression, sequence, declare, assign, Return,
          constant<std::size_t>, binop<'+'>, varref, list_test> {
private:
  using string_length = std::integral_constant<std::size_t, max_str_size>;
  using str_t = char const[string_length::value + 1];
  using str_nc = char[string_length::value + 1];
  template <std::size_t s> using fixed_cstr = mutils::cstring::fixed_cstr<s>;
  template <std::size_t s> using fixed_str = mutils::cstring::fixed_str<s>;

  template <std::size_t str_size>
  constexpr ct<expression> parse_add(const fixed_cstr<str_size> &_in) {
    using namespace mutils::cstring;
    str_nc operands[2] = {{0}};
    last_split('+', _in, operands);
    ct<expression> ret;
    auto dec = allocate<binop<'+'>>();
    deref(dec).FIELD(left) = parse_expression(operands[0]);
    deref(dec).FIELD(right) = parse_expression(operands[1]);
    ret.FIELD(arg) = upcast(std::move(dec));
    return ret;
  }

  template <std::size_t str_size>
  constexpr ct<expression> parse_constant(const fixed_cstr<str_size> &in) {
    using namespace mutils::cstring;
    ct<expression> ret;
    auto dec = allocate<constant<std::size_t>>();
    deref(dec).FIELD(value) = parse_int(in);
    ret.FIELD(arg) = upcast(std::move(dec));
    return ret;
  }

  template <std::size_t str_size>
  constexpr ct<expression> parse_varref(const fixed_cstr<str_size> &in) {
    using namespace mutils::cstring;
    ct<expression> ret;
    auto dec = allocate<varref>();
    trim(deref(dec).FIELD(var).strbuf, in);
    ret.FIELD(arg) = upcast(std::move(dec));
    return ret;
  }

  template <std::size_t str_size>
  constexpr ct<expression> parse_expression(const fixed_cstr<str_size> &_in) {
    using namespace mutils::cstring;
    fixed_str<str_size> in{0};
    trim(in, _in);
    if (contains_outside_parens("+", in)) {
      return parse_add(in);
    } else {
      // constants and variables here.
      str_nc atom = {0};
      trim(atom, in);
      static_assert('0' < '9');
      if (atom[0] >= '0' && atom[0] <= '9')
        return parse_constant(atom);
      else
        return parse_varref(atom);
    }
  }

  template <std::size_t str_size>
  constexpr ct<statement> parse_return(const fixed_cstr<str_size> &str) {
    using namespace mutils::cstring;
    ct<statement> ret;
    (MATCH(ret, stmt) {
      auto dec = allocate<Return>();
      str_nc ret_expr = {0};
      remove_first_word(ret_expr, str);
      ASSIGN_SINGLE(deref(dec), expr, parse_expression(ret_expr));
      stmt = upcast(std::move(dec));
    });
    return ret;
  }

  template <std::size_t str_size>
  constexpr ct<statement> parse_declare(const fixed_cstr<str_size> &str) {
    using namespace mutils::cstring;
    ct<statement> ret;
    ret.match([&](value::top_pointer & stmt) constexpr {
      auto dec = allocate<declare>();
      str_nc let_expr = {0};
      remove_first_word(let_expr, str);
      str_nc let_components[2] = {{0}};
      first_split(',', let_expr, let_components);
      deref(dec).match([&](auto &var, auto &expr, auto &body) constexpr {
        str_nc string_bufs[2] = {{0}};
        split_outside_parens('=', let_components[0], string_bufs);
        trim(var.strbuf, string_bufs[0]);
        expr = parse_expression(string_bufs[1]);
        body = parse_statement(let_components[1]);
      });
      stmt = upcast(std::move(dec));
    });
    return ret;
  }
  template <std::size_t str_size>
  constexpr ct<statement> parse_assignment(const fixed_cstr<str_size> &in) {
    using namespace mutils::cstring;
    ct<statement> ret;
    ret.match([&](value::top_pointer & stmt) constexpr {
      auto dec = allocate<assign>();
      deref(dec).match([&](auto &l, auto &r) constexpr {
        str_nc string_bufs[2] = {{0}};
        split_outside_parens('=', in, string_bufs);
        trim(l.strbuf, string_bufs[0]);
        r = parse_expression(string_bufs[1]);
      });
      stmt = upcast(std::move(dec));
    });
    return ret;
  }
  template <std::size_t str_size>
  constexpr ct<statement> parse_sequence(const fixed_cstr<str_size> &str) {
    using namespace mutils::cstring;
    str_nc string_bufs[2] = {{0}};
    first_split(',', str, string_bufs);
    ct<statement> ret;
    ret.match([&](value::top_pointer & stmt) constexpr {
      auto dec = allocate<sequence>();
      deref(dec).match([&](auto &fst, auto &rst) constexpr {
        fst = parse_statement(string_bufs[0]);
        rst = parse_statement(string_bufs[1]);
      });
      stmt = upcast(std::move(dec));
    });
    return ret;
  }

  template <std::size_t str_size>
  constexpr ct<statement> parse_statement(const fixed_cstr<str_size> &_in) {
    using namespace mutils::cstring;
    fixed_str<str_size> in{0};
    trim(in, _in);
    if (first_word_is("var", in)) {
      return parse_declare(in);
    } else if (contains_outside_parens(',', in)) {
      return parse_sequence(in);
    } else if (contains_outside_parens("=", in)) {
      return parse_assignment(in);
    } else if (first_word_is("return", in)) {
      return parse_return(in);
    } else {
      fixed_str<1024> error_str{0};
      combine_strings(error_str, "Failed to parse statement ", _in);
      error(error_str);
      return ct<statement>{};
    }
  }

public:
  constexpr ct<program> &parse_program(const fixed_cstr<max_str_size> &str) {
    current_return().match([ this, str ](ct<statement> & body) constexpr {
      auto ptr = allocate<list_test>();
      deref(ptr).FIELD(payload) = parse_statement(str);
      for (auto i = 0u; i < 10u; ++i)
        deref(ptr).FIELD(values).grow() = 5;
      body.FIELD(body) = upcast(std::move(ptr));
    });
    return current_return();
  }
  const ct<program> parsed;
  constexpr parser(const fixed_cstr<max_str_size> &str)
      : parsed(parse_program(str)) {}
};

template <char... c>
std::ostream &operator<<(std::ostream &o, const types::error<c...> &) {
  constexpr const char str[sizeof...(c) + 1] = {c..., 0};
  return o << str;
}

namespace type_printer {
using namespace types;

template <typename e> std::ostream &print(std::ostream &o, const e & = e{});

template <char... c>
std::ostream &_print(std::ostream &o, const types::error<c...> &e) {
  return o << e;
}

template <typename i, typename... v>
std::ostream &operator<<(std::ostream &o, const instance<i, v...> &o2) {
  return print(o, o2);
}

template <typename i, typename... v>
std::ostream &
_print(std::ostream &o,
       const instance<program, instance<statement, instance<i, v...>>> &) {
  return print(o, instance<i, v...>{});
}

template <typename i, typename... v>
std::ostream &_print(std::ostream &o,
                     const instance<statement, instance<i, v...>> &) {
  return print(o, instance<i, v...>{});
}

template <typename i, typename... v>
std::ostream &_print(std::ostream &o,
                     const instance<expression, instance<i, v...>> &) {
  return print(o, instance<i, v...>{});
}

std::ostream &_print(std::ostream &o, const instance<expression, null_type> &) {
  return print(o, null_type{});
}

std::ostream &_print(std::ostream &o, const null_type &) { return o << "NULL"; }

template <typename payload, typename... values>
std::ostream &_print(std::ostream &o,
                     const instance<list_test, list<values...>, payload> &) {
  o << "[";
  (print(o, values{}), ...);
  o << "]\n";
  return print(o, payload{});
}

template <typename T, T v>
std::ostream &_print(std::ostream &o,
                     const instance<constant<T>, raw_value<T, v>> &) {
  return o << v;
}

template <typename T, T v>
std::ostream &_print(std::ostream &o, const raw_value<T, v> &) {
  return o << v;
}

template <typename s1, typename s2>
std::ostream &_print(std::ostream &o, const instance<binop<'+'>, s1, s2> &) {
  return print(o, s1{}) << " + " << s2{};
}

template <typename v>
std::ostream &_print(std::ostream &o, const instance<varref, v> &) {
  return print(o, v{});
}

template <typename s1, typename s2>
std::ostream &_print(std::ostream &o, const instance<sequence, s1, s2> &) {
  return print(o, s1{}) << ", " << s2{};
}
template <typename e>
std::ostream &_print(std::ostream &o, const instance<Return, e> &) {
  return o << "return " << e{};
}

template <typename v, typename e, typename b>
std::ostream &_print(std::ostream &o, const instance<declare, v, e, b> &) {
  return o << "var " << v{} << " = " << e{} << ", " << b{};
}

template <typename v, typename e>
std::ostream &_print(std::ostream &o, const instance<assign, v, e> &) {
  return print(o, v{}) << " = " << e{};
}

template <char... c>
std::ostream &_print(std::ostream &o, const mutils::String<c...> &s) {
  return o << s.string;
}

template <typename e> std::ostream &print(std::ostream &o, const e &_e) {
  return _print(o, _e);
}
} // namespace type_printer

} // namespace simple_language
} // namespace compile_time

using namespace compile_time;
using namespace simple_language;

#define CONVERT_T(x...) compile_time_context<x>::template convert_to_type<x>
#define PARSE(x...)                                                            \
  parser<mutils::cstring::str_len(#x) + 1> { #x }

constexpr decltype(auto) parse_trial() {
  return PARSE(var x = 0, var y = 4, x = x + y, return x).allocator;
}

struct convert_parsed {
  static const constexpr DECT(parse_trial()) allocator{parse_trial()};
  static constexpr const DECT(allocator.top) &value = allocator.top;
  constexpr const auto &operator()() const { return value; }
  constexpr convert_parsed() = default;
};

using step1 = CONVERT_T(convert_parsed);
static_assert(step1::all_ok);
struct convert_again {
  static const constexpr DECT(parse_trial()) allocator{
      convert_to_value<DECT(parse_trial()), step1>()};
  static constexpr const DECT(allocator.top) &value = allocator.top;
  constexpr const auto &operator()() const { return value; }
  constexpr convert_again() = default;
};
using step2 = CONVERT_T(convert_again);
static_assert(std::is_same_v<step1, step2>, "Sanity check");

int main() {
  type_printer::print<step1>(std::cout) << std::endl << std::endl << std::endl;
  type_printer::print<step2>(std::cout) << std::endl;
}
