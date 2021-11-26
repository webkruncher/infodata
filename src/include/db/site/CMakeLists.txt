
file(GLOB includes ${CMAKE_CURRENT_LIST_DIR}/*.h)

install(
    FILES
	    ${includes}    
		    OPTIONAL
		    DESTINATION include/infokruncher/db/site
)

add_library(
    dbsite
	    INTERFACE
)

add_library( kruncher::db::site::headers ALIAS dbsite )

target_include_directories(
    dbsite
	    INTERFACE
		    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    dbsite
	    INTERFACE
)




