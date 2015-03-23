cmake_minimum_required(VERSION 2.8)
project(LockFreeDataStructures)

set(LFDS_VERSION_MAJOR 1)
set(LFDS_VERSION_MINOR 0)

configure_file("${PROJECT_SOURCE_DIR}/include/config.hpp.in" 
    "${PROJECT_SOURCE_DIR}/include/config.hpp")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)

include_directories(include)

if(CMAKE_COMPILER_IS_GNUCXX)
    message(status "GNUCXX detected")
    add_definitions(-std=c++11 -pthread)
endif()

enable_testing()
add_subdirectory(tests)
add_subdirectory(demo)
add_subdirectory(perftest)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include" DESTINATION "lfds")

set(CMAKE_CTEST_COMMAND ctest) 
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_test( LockFreeUnitTests bin/tests )
add_dependencies(check tests)

