#pragma once

#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace deque::detail {

template <class Allocator, class T, class... Args>
inline void constructAt(Allocator& allocator, T* ptr, Args&&... args) {
  std::allocator_traits<Allocator>::construct(allocator, ptr, std::forward<Args>(args)...);
}

template <class Allocator, class T>
inline void destroyAt(Allocator& allocator, T* ptr) noexcept {
  std::allocator_traits<Allocator>::destroy(allocator, ptr);
}

template <class Allocator>
inline auto allocateBlock(Allocator& allocator, std::size_t count)
    -> typename std::allocator_traits<Allocator>::pointer {
  return std::allocator_traits<Allocator>::allocate(allocator, count);
}

template <class Allocator>
inline void deallocateBlock(Allocator& allocator,
                            typename std::allocator_traits<Allocator>::pointer ptr,
                            std::size_t count) noexcept {
  if (ptr == nullptr) {
    return;
  }
  std::allocator_traits<Allocator>::deallocate(allocator, ptr, count);
}

}  // namespace deque::detail
