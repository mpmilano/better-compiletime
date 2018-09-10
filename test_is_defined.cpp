#include "utils.hpp"

using namespace mutils;

class A {};
class B;

static_assert(is_defined<A>::value);
static_assert(!is_defined<B>::value);
static_assert(is_defined<int>::value);

int main() {}