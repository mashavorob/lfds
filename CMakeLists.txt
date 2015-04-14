cmake_minimum_required(VERSION 2.8)
project(LockFreeDataStructures)

set(LFDS_VERSION_MAJOR 1)
set(LFDS_VERSION_MINOR 0)

if(CMAKE_COMPILER_IS_GNUCXX)
    message(status "GNUCXX detected")

    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
        message(STATUS "Version >= 4.8")
        add_definitions(-std=c++11)
        set(LFDS_USE_CPP11 1)
    else()
        set(LFDS_USE_CPP11 0)
    endif()    
    add_definitions(-pthread)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

configure_file("${PROJECT_SOURCE_DIR}/include/config.hpp.in" 
    "${PROJECT_SOURCE_DIR}/include/config.hpp")

include_directories(
    include
    utils
    )

enable_testing()
add_subdirectory(tests)
if ( LFDS_USE_CPP11 )
    add_subdirectory(demo)
endif()
add_subdirectory(perftest)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include" DESTINATION "lfds")

set(CMAKE_CTEST_COMMAND ctest) 
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_test( LockFreeUnitTests bin/unit-tests )
add_dependencies(check tests)

