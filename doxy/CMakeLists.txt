find_package( Doxygen )
if( DOXYGEN_FOUND )

    set( XTOMIC_PUBLIC_INTERFACE ${PROJECT_SOURCE_DIR}/include/xtomic )
    set( XTOMIC_MAIN_PAGE ${CMAKE_CURRENT_SOURCE_DIR}/mainpage.md )
    set( 
        XTOMIC_IMPLEMENTATION_DETAILS 
        ${PROJECT_SOURCE_DIR}/include/xtomic/impl
        ${PROJECT_SOURCE_DIR}/include/xtomic/config.hpp
    )
    
    string( REPLACE ";" " " XTOMIC_IMPLEMENTATION_DETAILS "${XTOMIC_IMPLEMENTATION_DETAILS}" )
    
    set( XTOMIC_DOCUMENTATION ${CMAKE_BINARY_DIR}/doc )
    
    configure_file("${PROJECT_SOURCE_DIR}/doxy/config.in" 
        "${CMAKE_CURRENT_BINARY_DIR}/config")
    add_custom_target(doxy COMMAND ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/config)
else()
    message(STATUS "WARNING: Doxygen not found - Reference manual will not be created")
endif(DOXYGEN_FOUND)
