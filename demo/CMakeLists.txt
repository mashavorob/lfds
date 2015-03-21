cmake_minimum_required(VERSION 2.8)

################################
# Demo
################################
# Add test cpp file

find_package (Threads)

add_executable( demo
    demo.cpp 
    demo_queue.cpp 
    demo_stack.cpp 
    casbenchmark.cpp
    virtualbenchmark.cpp
    demo_map.cpp
    demo_set.cpp
    random_strings.cpp
    )

target_link_libraries (demo ${CMAKE_THREAD_LIBS_INIT})

#just a smoke test
add_test(DemoSmokeTest ./../bin/demo)

#usage information
add_test(DemoUsageString ./../bin/demo --help)
set_tests_properties(DemoUsageString PROPERTIES PASS_REGULAR_EXPRESSION "Usage:.*message")

install(TARGETS demo DESTINATION lfds/bin)
