
Banner()
file(GLOB includes ${CMAKE_CURRENT_LIST_DIR}/*.h)

install(
    FILES
	    ${includes}    
		    OPTIONAL
		    DESTINATION include/infokruncher/infodata/db/auth
)

add_library(
    dbauth
	    INTERFACE
)

add_library( kruncher::db::auth::headers ALIAS dbauth )

target_include_directories(
    dbauth
	    INTERFACE
		    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    dbauth
	    INTERFACE
)




