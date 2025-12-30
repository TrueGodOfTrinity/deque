add_executable(deque_tests
  test_main.cpp
  test_api_basic.cpp
  test_iterators.cpp
  test_modifiers.cpp
  test_compare.cpp
  test_vs_std_deque.cpp
)

target_link_libraries(deque_tests PRIVATE deque)

target_compile_features(deque_tests PRIVATE cxx_std_17)

if (MSVC)
  target_compile_options(deque_tests PRIVATE /W4 /permissive-)
else()
  target_compile_options(deque_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME deque_tests COMMAND deque_tests)
