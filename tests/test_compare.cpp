#include <cassert>

#include "deque/deque.hpp"

void runCompareTests() {
  deque::Deque<int> a;
  deque::Deque<int> b;

  assert(a == b);
  assert(!(a != b));
  assert(!(a < b));
  assert(a <= b);
  assert(a >= b);

  a.pushBack(1);
  a.pushBack(2);
  b.pushBack(1);
  b.pushBack(3);

  assert(a != b);
  assert(a < b);
  assert(!(a > b));

  b.popBack();
  b.pushBack(2);

  assert(a == b);
}
