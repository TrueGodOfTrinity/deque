//验证迭代器体系（随机访问迭代器、反向迭代器）
#include <cassert>
#include <numeric>
#include <vector>

#include "deque/deque.hpp"

void runIteratorTests() {
  deque::Deque<int> d;
  for (int i = 0; i < 1000; ++i) {
    d.pushBack(i);
  }

  //前向迭代
  long long sum_1 = 0;
  for (auto it = d.begin(); it != d.end(); ++it) {
    sum_1 += *it;
  }

  long long sum_2 = 0;
  for (int i = 0; i < 1000; ++i) {
    sum_2 += i;
  }
  assert(sum_1 == sum_2);

  //随机访问
  auto it = d.begin();
  it += 10;
  assert(*it == 10);
  assert(it[5] == 15);
  assert((d.end() - d.begin()) == static_cast<std::ptrdiff_t>(d.size()));

  // 反向迭代
  int last = 999;
  for (auto rit = d.rBegin(); rit != d.rEnd(); ++rit) {
    assert(*rit == last);
    --last;
  }
  assert(last == -1);

  // 常量迭代器
  deque::Deque<int>::const_iterator cit = d.begin();
  assert(*cit == 0);
}
