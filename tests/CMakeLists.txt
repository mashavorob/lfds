cmake_minimum_required(VERSION 2.8)


# Google Test Framework
add_subdirectory (gtest-1.7.0)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})
add_subdirectory (units)
