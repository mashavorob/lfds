cmake_minimum_required(VERSION 2.8)
project(xtomic)

set(XTOMIC_VERSION_MAJOR 1)
set(XTOMIC_VERSION_MINOR 0)

if(CMAKE_COMPILER_IS_GNUCXX)
    message(status "GNUCXX detected")

    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_GREATER 4.8 OR GCC_VERSION VERSION_EQUAL 4.8)
        message(STATUS "Version >= 4.8")
        add_definitions(-std=c++11)
        set(XTOMIC_USE_CPP11 1)
    else()
        set(XTOMIC_USE_CPP11 0)
    endif()    
    add_definitions(-pthread)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/include/xtomic/config.hpp.in" 
    "${CMAKE_CURRENT_BINARY_DIR}/include/xtomic/config.hpp")

include_directories(
    include
    utils
    ${CMAKE_CURRENT_BINARY_DIR}/include
    )

enable_testing()
add_subdirectory(tests)
add_subdirectory(perftest)
add_subdirectory(doxy)

install(DIRECTORY "${PROJECT_SOURCE_DIR}/include" DESTINATION "lfds")

set(CMAKE_CTEST_COMMAND ctest) 
add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND})
add_test( xtomic-unit-tests bin/unit-tests )
add_dependencies(check tests)

