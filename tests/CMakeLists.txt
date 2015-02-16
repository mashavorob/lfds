cmake_minimum_required(VERSION 2.8)

ADD_SUBDIRECTORY (gtest-1.7.0)
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

################################
# Unit Tests
################################
# Add test cpp file
add_executable( tests 
    cas.cpp 
    abaptr.cpp
    stack_base_weak.cpp
    stack_base_aba.cpp
    queue_base.cpp
    buffers.cpp
    stack.cpp
    queue.cpp
    hash_map.cpp
    hash_set.cpp
    hash_map_integral_key.cpp
    hash_map_integral_pair.cpp
    hash_set_integral_key.cpp
    )

# Link test executable against gtest & gtest_main
target_link_libraries(tests gtest gtest_main)
