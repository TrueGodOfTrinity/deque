#pragma once

#include <cstddef>
#include <iterator>
#include <type_traits>

namespace deque::detail {

template <class StorageType, bool is_const>
class DequeIterator {
 public:
  using storage_type = StorageType;

  using value_type = typename storage_type::value_type;
  using difference_type = typename storage_type::difference_type;

  using reference = std::conditional_t<is_const, const value_type&, value_type&>;
  using pointer = std::conditional_t<is_const, const value_type*, value_type*>;

  using iterator_category = std::random_access_iterator_tag;

  DequeIterator() = default;
  //explicit:防止构造函数内容隐式转换
  explicit DequeIterator(std::conditional_t<is_const, const storage_type*, storage_type*> storage, std::size_t index)
      : storage_(storage), index_(index) {}

  //让非const 迭代器能够隐式转换为 const 迭代器
  template <bool other_const, class = std::enable_if_t<is_const && !other_const>>
  DequeIterator(const DequeIterator<storage_type, other_const>& other)
      : storage_(other.storage_), index_(other.index_) {}

  reference operator*() const { return storage_->atIndex(index_); }
  pointer operator->() const { return &storage_->atIndex(index_); }

  DequeIterator& operator++() {
    ++index_;
    return *this;
  }

  DequeIterator operator++(int) {
    DequeIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  DequeIterator& operator--() {
    --index_;
    return *this;
  }

  DequeIterator operator--(int) {
    DequeIterator tmp = *this;
    --(*this);
    return tmp;
  }

  DequeIterator& operator+=(difference_type n) {
    index_ = static_cast<std::size_t>(static_cast<difference_type>(index_) + n);
    return *this;
  }

  DequeIterator& operator-=(difference_type n) {
    index_ = static_cast<std::size_t>(static_cast<difference_type>(index_) - n);
    return *this;
  }

  DequeIterator operator+(difference_type n) const {
    DequeIterator tmp = *this;
    tmp += n;
    return tmp;
  }

  DequeIterator operator-(difference_type n) const {
    DequeIterator tmp = *this;
    tmp -= n;
    return tmp;
  }

  difference_type operator-(const DequeIterator& other) const {
    return static_cast<difference_type>(index_) - static_cast<difference_type>(other.index_);
  }

  reference operator[](difference_type n) const { return *(*this + n); }

  bool operator==(const DequeIterator& other) const { return storage_ == other.storage_ && index_ == other.index_; }
  bool operator!=(const DequeIterator& other) const { return !(*this == other); }

  bool operator<(const DequeIterator& other) const { return index_ < other.index_; }
  bool operator<=(const DequeIterator& other) const { return index_ <= other.index_; }
  bool operator>(const DequeIterator& other) const { return index_ > other.index_; }
  bool operator>=(const DequeIterator& other) const { return index_ >= other.index_; }

  std::size_t getIndex() const noexcept { return index_; }

 private:
  template <class, bool>
  friend class DequeIterator;

  std::conditional_t<is_const, const storage_type*, storage_type*> storage_ = nullptr;
  std::size_t index_ = 0;
};

template <class StorageType, bool is_const>
inline DequeIterator<StorageType, is_const> operator+(typename DequeIterator<StorageType, is_const>::difference_type n,
                                                     const DequeIterator<StorageType, is_const>& it) {
  return it + n;
}

}  // namespace deque::detail
