#include <cassert>
#include <vector>

#include "deque/deque.hpp"

void runModifierTests() {
  deque::Deque<int> d;

  d.assign(5, 7);
  assert(d.size() == 5);
  for (std::size_t i = 0; i < d.size(); ++i) {
    assert(d[i] == 7);
  }

  // insert in middle
  auto it = d.begin();
  it += 2;
  d.insert(it, 42);
  assert(d.size() == 6);
  assert(d[2] == 42);

  // erase single
  d.erase(d.begin() + 2);
  assert(d.size() == 5);
  assert(d[2] == 7);

  // erase range
  d.insert(d.begin(), 1);
  d.insert(d.begin(), 2);
  // now: 2,1,7,7,7,7,7
  assert(d.size() == 7);
  d.erase(d.begin() + 1, d.begin() + 3); // remove 1 and first 7
  assert(d.size() == 5);
  assert(d.front() == 2);

  // resize
  d.resize(10, 9);
  assert(d.size() == 10);
  for (std::size_t i = 5; i < 10; ++i) {
    assert(d[i] == 9);
  }

  d.resize(3);
  assert(d.size() == 3);

  // swap
  deque::Deque<int> other;
  other.pushBack(100);
  other.pushBack(200);

  d.swap(other);
  assert(d.size() == 2);
  assert(d[0] == 100 && d[1] == 200);
  assert(other.size() == 3);
}
