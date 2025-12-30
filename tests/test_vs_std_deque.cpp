//随机对拍（property-based / differential testing），系统验证正确性
#include <cassert>
#include <cstddef>
#include <deque>
#include <random>
#include <vector>

#include "deque/deque.hpp"

static void assertSame(const deque::Deque<int>& my_deque, const std::deque<int>& std_deque) {
  assert(my_deque.size() == std_deque.size());
  for (std::size_t i = 0; i < my_deque.size(); ++i) {
    assert(my_deque[i] == std_deque[i]);
  }
}

void runVsStdDequeTests() {
  deque::Deque<int> my_deque;
  std::deque<int> std_deque;

  std::mt19937 rng(12345);
  std::uniform_int_distribution<int> op_dist(0, 7);
  std::uniform_int_distribution<int> val_dist(-1000, 1000);

  for (int step = 0; step < 5000; ++step) {
    int op = op_dist(rng);

    if (op == 0) {  // pushBack
      int value = val_dist(rng);
      my_deque.pushBack(value);
      std_deque.push_back(value);
    } else if (op == 1) {  // pushFront
      int value = val_dist(rng);
      my_deque.pushFront(value);
      std_deque.push_front(value);
    } else if (op == 2) {  // popBack
      if (!std_deque.empty()) {
        my_deque.popBack();
        std_deque.pop_back();
      }
    } else if (op == 3) {  // popFront
      if (!std_deque.empty()) {
        my_deque.popFront();
        std_deque.pop_front();
      }
    } else if (op == 4) {  // insert
      int value = val_dist(rng);
      std::size_t pos = std_deque.empty() ? 0 : static_cast<std::size_t>(rng() % (std_deque.size() + 1));

      my_deque.insert(my_deque.begin() + static_cast<std::ptrdiff_t>(pos), value);
      std_deque.insert(std_deque.begin() + static_cast<std::ptrdiff_t>(pos), value);
    } else if (op == 5) {  // erase
      if (!std_deque.empty()) {
        std::size_t pos = static_cast<std::size_t>(rng() % std_deque.size());
        my_deque.erase(my_deque.begin() + static_cast<std::ptrdiff_t>(pos));
        std_deque.erase(std_deque.begin() + static_cast<std::ptrdiff_t>(pos));
      }
    } else if (op == 6) {  // resize
      std::size_t new_size = static_cast<std::size_t>(rng() % 200);
      int value = val_dist(rng);
      my_deque.resize(new_size, value);
      std_deque.resize(new_size, value);
    } else {  // clear
      if ((rng() % 50) == 0) {
        my_deque.clear();
        std_deque.clear();
      }
    }

    assertSame(my_deque, std_deque);
  }
}
