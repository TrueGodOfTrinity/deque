#pragma once

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "deque/detail/storage.hpp"
#include "deque/detail/iterator.hpp"

namespace deque {

// Deque container with segmented storage.
// API uses lowerCamelCase function naming by request.
template <class T, class Allocator = std::allocator<T>>
class Deque {
 public:
  using value_type = T;
  using allocator_type = Allocator;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

 private:
  using storage_type = detail::SegmentedStorage<T, Allocator>;

 public:
  using iterator = detail::DequeIterator<storage_type, false>;
  using const_iterator = detail::DequeIterator<storage_type, true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  Deque() = default;

  Deque(const Deque& other) = default;
  Deque(Deque&& other) noexcept = default;

  Deque& operator=(Deque other) noexcept(std::is_nothrow_move_constructible_v<storage_type>) {
    swap(other);
    return *this;
  }

  ~Deque() = default;

  void swap(Deque& other) noexcept { storage_.swap(other.storage_); }

  bool empty() const noexcept { return storage_.empty(); }
  size_type size() const noexcept { return storage_.size(); }

  void clear() { storage_.clear(); }

  iterator begin() noexcept { return iterator(&storage_, 0); }
  const_iterator begin() const noexcept { return const_iterator(&storage_, 0); }
  const_iterator cbegin() const noexcept { return const_iterator(&storage_, 0); }

  iterator end() noexcept { return iterator(&storage_, storage_.size()); }
  const_iterator end() const noexcept { return const_iterator(&storage_, storage_.size()); }
  const_iterator cend() const noexcept { return const_iterator(&storage_, storage_.size()); }

  reverse_iterator rBegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rBegin() const noexcept { return const_reverse_iterator(end()); }

  reverse_iterator rEnd() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rEnd() const noexcept { return const_reverse_iterator(begin()); }

  value_type& front() { return storage_.front(); }
  const value_type& front() const { return storage_.front(); }

  value_type& back() { return storage_.back(); }
  const value_type& back() const { return storage_.back(); }

  value_type& operator[](size_type index) { return storage_.atIndex(index); }
  const value_type& operator[](size_type index) const { return storage_.atIndex(index); }

  void pushBack(const value_type& value) { storage_.pushBack(value); }
  void pushBack(value_type&& value) { storage_.pushBack(std::move(value)); }

  void pushFront(const value_type& value) { storage_.pushFront(value); }
  void pushFront(value_type&& value) { storage_.pushFront(std::move(value)); }

  void popBack() { storage_.popBack(); }
  void popFront() { storage_.popFront(); }

  iterator insert(const_iterator pos, const value_type& value) {
    size_type index = pos.getIndex();
    size_type inserted = storage_.insertAt(index, value);
    return iterator(&storage_, inserted);
  }

  iterator erase(const_iterator pos) {
    size_type index = pos.getIndex();
    size_type next_index = storage_.eraseAt(index);
    return iterator(&storage_, next_index);
  }

  iterator erase(const_iterator first, const_iterator last) {
    size_type first_index = first.getIndex();
    size_type last_index = last.getIndex();
    size_type next_index = storage_.eraseRange(first_index, last_index);
    return iterator(&storage_, next_index);
  }

  void resize(size_type count) { storage_.resize(count); }
  void resize(size_type count, const value_type& value) { storage_.resize(count, value); }

  void assign(size_type count, const value_type& value) { storage_.assign(count, value); }

  template <class InputIt, class = std::enable_if_t<!std::is_integral_v<InputIt>>>
  void assign(InputIt first, InputIt last) { storage_.assign(first, last); }

  friend bool operator==(const Deque& lhs, const Deque& rhs) {
    if (lhs.size() != rhs.size()) {
      return false;
    }
    for (size_type i = 0; i < lhs.size(); ++i) {
      if (!(lhs[i] == rhs[i])) {
        return false;
      }
    }
    return true;
  }

  friend bool operator!=(const Deque& lhs, const Deque& rhs) { return !(lhs == rhs); }

  friend bool operator<(const Deque& lhs, const Deque& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
  }

  friend bool operator<=(const Deque& lhs, const Deque& rhs) { return !(rhs < lhs); }
  friend bool operator>(const Deque& lhs, const Deque& rhs) { return rhs < lhs; }
  friend bool operator>=(const Deque& lhs, const Deque& rhs) { return !(lhs < rhs); }

 private:
  storage_type storage_{};
};

template <class T, class Allocator>
inline void swap(Deque<T, Allocator>& lhs, Deque<T, Allocator>& rhs) noexcept {
  lhs.swap(rhs);
}

}  // namespace deque
