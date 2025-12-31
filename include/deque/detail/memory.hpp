#pragma once

#include <memory>   //std::allocator_traits
#include <new>      //placement new
#include <type_traits>   //std::is_nothrow_destructible
#include <utility>     //std::forward

namespace deque::detail {

//分配、构造、销毁、释放内存的辅助函数
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
