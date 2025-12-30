add_library(deque STATIC deque_dummy.cpp)

target_include_directories(deque
  PUBLIC
    ${PROJECT_SOURCE_DIR}/include
)

target_compile_features(deque PUBLIC cxx_std_17)

if (MSVC)
  target_compile_options(deque PRIVATE /W4 /permissive-)
else()
  target_compile_options(deque PRIVATE -Wall -Wextra -Wpedantic)
endif()
