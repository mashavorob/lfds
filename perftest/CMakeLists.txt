find_package (Threads)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable( perftest 
	cmdlineparser.cpp
	performancetest.cpp
	perftest.cpp
	testfactory.cpp
	testallocator.cpp
	testfilter.cpp
	testlocator.cpp
	testmaxtime.cpp
	testmem.cpp
	testmultithread.cpp
	testtree.cpp
	timeutils.cpp
	wildcard.cpp
	maps/maptests.cpp
	)
	
target_link_libraries (
    perftest
    ${CMAKE_THREAD_LIBS_INIT}
    rt
    )

install(
  PROGRAMS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/perftest
  DESTINATION ${XTOMIC_INSTALL_PREFIX}/bin
  )
	