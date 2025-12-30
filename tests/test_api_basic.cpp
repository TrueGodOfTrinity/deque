#include <cassert>

#include "deque/deque.hpp"

void runApiBasicTests() {
  deque::Deque<int> d;

  assert(d.empty());
  assert(d.size() == 0);

  d.pushBack(1);
  d.pushBack(2);
  d.pushFront(0);

  assert(!d.empty());
  assert(d.size() == 3);
  assert(d.front() == 0);
  assert(d.back() == 2);

  assert(d[0] == 0);
  assert(d[1] == 1);
  assert(d[2] == 2);

  d.popFront();
  assert(d.front() == 1);
  d.popBack();
  assert(d.back() == 1);
  assert(d.size() == 1);

  d.clear();
  assert(d.empty());
}
