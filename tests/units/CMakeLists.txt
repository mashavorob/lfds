cmake_minimum_required(VERSION 2.8)

################################
# Unit Tests
################################
# Add test cpp file
add_executable( unit-tests 
    meta_test.cpp
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
    hash_map_integral_value.cpp
    hash_map_integral_pair.cpp
    hash_set_integral_key.cpp
    hash_map_simplified.cpp
    hash_trie.cpp
    stack_node.cpp
    queue_one2one.cpp
    queue_many2many.cpp
    )

# Link test executable against gtest & gtest_main
target_link_libraries(unit-tests gtest gtest_main)
