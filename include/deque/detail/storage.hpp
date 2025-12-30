#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "deque/detail/memory.hpp"

namespace deque::detail {

// A segmented storage similar to std::deque's model:
// - data stored in fixed-size blocks
// - an index array ("map") stores pointers to blocks
// - start/finish are cursors into the segmented space
template <class T, class Allocator = std::allocator<T>>
class SegmentedStorage {
 public:
  using value_type = T;
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;

  using allocator_type = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
  using allocator_traits = std::allocator_traits<allocator_type>;

  using map_allocator_type = std::allocator<T*>;
  using map_allocator_traits = std::allocator_traits<map_allocator_type>;

  static constexpr size_type block_size = 64;

  SegmentedStorage() {
    initEmpty_();
  }

  SegmentedStorage(const SegmentedStorage& other) : allocator_(allocator_traits::select_on_container_copy_construction(other.allocator_)) {
    initEmpty_();
    try {
      for (size_type i = 0; i < other.size_; ++i) {
        pushBack(other.atIndex(i));
      }
    } catch (...) {
      destroyAll_();
      freeAllBlocks_();
      freeMap_();
      throw;
    }
  }

  SegmentedStorage(SegmentedStorage&& other) noexcept
      : map_(other.map_),
        map_capacity_(other.map_capacity_),
        start_block_(other.start_block_),
        start_offset_(other.start_offset_),
        finish_block_(other.finish_block_),
        finish_offset_(other.finish_offset_),
        size_(other.size_),
        allocator_(std::move(other.allocator_)) {
    other.map_ = nullptr;
    other.map_capacity_ = 0;
    other.start_block_ = 0;
    other.start_offset_ = 0;
    other.finish_block_ = 0;
    other.finish_offset_ = 0;
    other.size_ = 0;
  }

  SegmentedStorage& operator=(SegmentedStorage other) noexcept(std::is_nothrow_move_constructible_v<allocator_type>) {
    swap(other);
    return *this;
  }

  ~SegmentedStorage() {
    destroyAll_();
    freeAllBlocks_();
    freeMap_();
  }

  void swap(SegmentedStorage& other) noexcept {
    using std::swap;
    swap(map_, other.map_);
    swap(map_capacity_, other.map_capacity_);
    swap(start_block_, other.start_block_);
    swap(start_offset_, other.start_offset_);
    swap(finish_block_, other.finish_block_);
    swap(finish_offset_, other.finish_offset_);
    swap(size_, other.size_);
    swap(allocator_, other.allocator_);
  }

  bool empty() const noexcept { return size_ == 0; }
  size_type size() const noexcept { return size_; }

  void clear() {
    destroyAll_();
    // Keep one central block allocated for future growth, free the rest.
    resetToCenter_();
  }

  T& atIndex(size_type index) {
    assert(index < size_);
    auto [block_index, offset] = locate_(index);
    return map_[block_index][offset];
  }

  const T& atIndex(size_type index) const {
    assert(index < size_);
    auto [block_index, offset] = locate_(index);
    return map_[block_index][offset];
  }

  T& front() {
    assert(size_ > 0);
    return atIndex(0);
  }

  const T& front() const {
    assert(size_ > 0);
    return atIndex(0);
  }

  T& back() {
    assert(size_ > 0);
    return atIndex(size_ - 1);
  }

  const T& back() const {
    assert(size_ > 0);
    return atIndex(size_ - 1);
  }

  void pushBack(const T& value) { emplaceBack_(value); }
  void pushBack(T&& value) { emplaceBack_(std::move(value)); }

  void pushFront(const T& value) { emplaceFront_(value); }
  void pushFront(T&& value) { emplaceFront_(std::move(value)); }

  void popBack() {
    assert(size_ > 0);
    decrementFinish_();
    T* ptr = elementPtr_(finish_block_, finish_offset_);
    destroyAt(allocator_, ptr);
    --size_;
    if (size_ == 0) {
      // keep cursors aligned
      start_block_ = finish_block_;
      start_offset_ = finish_offset_;
    }
  }

  void popFront() {
    assert(size_ > 0);
    T* ptr = elementPtr_(start_block_, start_offset_);
    destroyAt(allocator_, ptr);
    incrementStart_();
    --size_;
    if (size_ == 0) {
      finish_block_ = start_block_;
      finish_offset_ = start_offset_;
    }
  }

  // Insert value at index (0..size). Returns the index of inserted element.
  size_type insertAt(size_type index, const T& value) {
    assert(index <= size_);
    if (index == size_) {
      pushBack(value);
      return size_ - 1;
    }
    if (size_ == 0) {
      pushBack(value);
      return 0;
    }

    // Create an extra slot at end by duplicating last element, then shift.
    pushBack(back());
    for (size_type i = size_ - 1; i > index; --i) {
      atIndex(i) = std::move(atIndex(i - 1));
    }
    atIndex(index) = value;
    return index;
  }

  // Erase element at index (0..size-1). Returns index of next element (same index).
  size_type eraseAt(size_type index) {
    assert(index < size_);
    for (size_type i = index; i + 1 < size_; ++i) {
      atIndex(i) = std::move(atIndex(i + 1));
    }
    popBack();
    return index;
  }

  // Erase [first, last). Returns index of first.
  size_type eraseRange(size_type first, size_type last) {
    assert(first <= last);
    assert(last <= size_);
    size_type count = last - first;
    if (count == 0) {
      return first;
    }
    for (size_type i = first; i + count < size_; ++i) {
      atIndex(i) = std::move(atIndex(i + count));
    }
    for (size_type k = 0; k < count; ++k) {
      popBack();
    }
    return first;
  }

  void resize(size_type count) {
    if (count < size_) {
      while (size_ > count) {
        popBack();
      }
      return;
    }
    while (size_ < count) {
      emplaceBack_(T{});
    }
  }

  void resize(size_type count, const T& value) {
    if (count < size_) {
      while (size_ > count) {
        popBack();
      }
      return;
    }
    while (size_ < count) {
      pushBack(value);
    }
  }

  void assign(size_type count, const T& value) {
    clear();
    for (size_type i = 0; i < count; ++i) {
      pushBack(value);
    }
  }

  template <class InputIt, class = std::enable_if_t<!std::is_integral_v<InputIt>>>
  void assign(InputIt first, InputIt last) {
    clear();
    for (auto it = first; it != last; ++it) {
      pushBack(*it);
    }
  }

 private:
  struct Location {
    size_type block_index;
    size_type offset;
  };

  T** map_ = nullptr;
  size_type map_capacity_ = 0;

  size_type start_block_ = 0;
  size_type start_offset_ = 0;

  size_type finish_block_ = 0;
  size_type finish_offset_ = 0;

  size_type size_ = 0;

  allocator_type allocator_{};

  void initEmpty_() {
    map_capacity_ = 8;
    map_ = map_allocator_traits::allocate(map_allocator_, map_capacity_);
    for (size_type i = 0; i < map_capacity_; ++i) {
      map_[i] = nullptr;
    }

    start_block_ = map_capacity_ / 2;
    finish_block_ = start_block_;

    allocateBlockIfNeeded_(start_block_);

    start_offset_ = block_size / 2;
    finish_offset_ = start_offset_;
    size_ = 0;
  }

  void resetToCenter_() {
    // Destroyed already, so only deallocate blocks and reinit
    freeAllBlocks_();
    freeMap_();
    initEmpty_();
  }

  void destroyAll_() noexcept {
    // Destroy in logical order to avoid double-destruction hazards.
    for (size_type i = 0; i < size_; ++i) {
      auto [block_index, offset] = locate_(i);
      destroyAt(allocator_, elementPtr_(block_index, offset));
    }
    size_ = 0;
    finish_block_ = start_block_;
    finish_offset_ = start_offset_;
  }

  void freeAllBlocks_() noexcept {
    if (map_ == nullptr) {
      return;
    }
    for (size_type i = 0; i < map_capacity_; ++i) {
      if (map_[i] != nullptr) {
        deallocateBlock(allocator_, map_[i], block_size);
        map_[i] = nullptr;
      }
    }
  }

  void freeMap_() noexcept {
    if (map_ == nullptr) {
      return;
    }
    map_allocator_traits::deallocate(map_allocator_, map_, map_capacity_);
    map_ = nullptr;
    map_capacity_ = 0;
  }

  void allocateBlockIfNeeded_(size_type block_index) {
    assert(block_index < map_capacity_);
    if (map_[block_index] == nullptr) {
      map_[block_index] = allocateBlock(allocator_, block_size);
    }
  }

  void growMapIfNeeded_(bool grow_front) {
    if (!grow_front) {
      if (finish_block_ + 1 < map_capacity_) {
        return;
      }
    } else {
      if (start_block_ > 0) {
        return;
      }
    }

    size_type new_capacity = map_capacity_ * 2;
    T** new_map = map_allocator_traits::allocate(map_allocator_, new_capacity);
    for (size_type i = 0; i < new_capacity; ++i) {
      new_map[i] = nullptr;
    }

    // Re-center existing pointers.
    size_type used_begin = start_block_;
    size_type used_end = finish_block_;
    size_type used_count = (used_end - used_begin) + 1;

    size_type new_begin = (new_capacity - used_count) / 2;
    for (size_type i = 0; i < used_count; ++i) {
      new_map[new_begin + i] = map_[used_begin + i];
    }

    map_allocator_traits::deallocate(map_allocator_, map_, map_capacity_);

    start_block_ = new_begin;
    finish_block_ = new_begin + used_count - 1;

    map_ = new_map;
    map_capacity_ = new_capacity;
  }

  Location locate_(size_type index) const {
    size_type absolute = start_offset_ + index;
    size_type block_shift = absolute / block_size;
    size_type offset = absolute % block_size;
    size_type block_index = start_block_ + block_shift;
    return {block_index, offset};
  }

  T* elementPtr_(size_type block_index, size_type offset) const {
    return map_[block_index] + offset;
  }

  void incrementStart_() {
    ++start_offset_;
    if (start_offset_ == block_size) {
      start_offset_ = 0;
      ++start_block_;
      assert(start_block_ < map_capacity_);
      allocateBlockIfNeeded_(start_block_);
    }
  }

  void decrementFinish_() {
    if (finish_offset_ == 0) {
      assert(finish_block_ > 0);
      --finish_block_;
      finish_offset_ = block_size;
    }
    --finish_offset_;
  }

  template <class U>
  void emplaceBack_(U&& value) {
    growMapIfNeeded_(false);

    if (finish_offset_ == block_size) {
      finish_offset_ = 0;
      ++finish_block_;
      assert(finish_block_ < map_capacity_);
      allocateBlockIfNeeded_(finish_block_);
    }

    T* ptr = elementPtr_(finish_block_, finish_offset_);
    constructAt(allocator_, ptr, std::forward<U>(value));
    ++finish_offset_;
    ++size_;
  }

  template <class U>
  void emplaceFront_(U&& value) {
    growMapIfNeeded_(true);

    if (start_offset_ == 0) {
      assert(start_block_ > 0);
      --start_block_;
      allocateBlockIfNeeded_(start_block_);
      start_offset_ = block_size;
    }
    --start_offset_;

    T* ptr = elementPtr_(start_block_, start_offset_);
    constructAt(allocator_, ptr, std::forward<U>(value));
    ++size_;
  }

  map_allocator_type map_allocator_{};
};

}  // namespace deque::detail
