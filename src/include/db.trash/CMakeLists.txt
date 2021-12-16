
file(GLOB includes ${CMAKE_CURRENT_LIST_DIR}/*.h)

install(
    FILES
	    ${includes}    
		    OPTIONAL
		    DESTINATION include/infokruncher/infodata/db
)

add_library(
    db
	    INTERFACE
)

add_library( kruncher::db ALIAS db )

target_include_directories(
    db
	    INTERFACE
		    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    db
	    INTERFACE
)



add_subdirectory( records ) 
add_subdirectory( auth )
add_subdirectory( site )
