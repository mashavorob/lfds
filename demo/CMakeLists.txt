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
	)

target_link_libraries (demo ${CMAKE_THREAD_LIBS_INIT})
