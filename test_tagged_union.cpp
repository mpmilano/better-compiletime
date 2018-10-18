#include "tagged_union.hpp"

using namespace compile_time;

constexpr int test() {
  tagged_union<int, double, float> tu;
  tu.template reset<int>() = 4;
  return tu.template get<int>();
}

int main() {
  constexpr int test_result = test();
  static_assert(test_result == 4, "");
}