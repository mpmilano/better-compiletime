#include "tagged_union.hpp"
#include <cassert>

using namespace compile_time;

constexpr int test() {
  tagged_union<int, double, float> tu;
  tu.template reset<int>() = 4;
  return tu.template get<int>();
}

int main() {
  constexpr int test_result = test();
  tagged_union<int, double, float> tu;
  tu.template reset<int>() = 4;
  tagged_union<int, double, float> tu2 = tu;
  tu2.template reset<float>() = 4;
  assert(tu2.template get<float>() == tu.template get<int>());
  static_assert(test_result == 4, "");
}