cmake_minimum_required(VERSION 3.16)

project(deque_project VERSION 0.1.0 LANGUAGES CXX)

option(DEQUE_BUILD_TESTS "Build deque tests" ON)

add_subdirectory(src)

if(DEQUE_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
