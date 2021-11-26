
file(GLOB includes ${CMAKE_CURRENT_LIST_DIR}/*.h)


install(
    FILES
	    ${includes}    
		    OPTIONAL
		    DESTINATION include/infokruncher/infodata/db/records
)

add_library(
    dbrecords
	    INTERFACE
)

add_library( infodata::db::records::headers ALIAS dbrecords )

target_include_directories(
    dbrecords
	    INTERFACE
		    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    dbrecords
	    INTERFACE
)




