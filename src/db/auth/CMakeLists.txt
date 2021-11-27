
message("Building infodata/db/auth")


set (LIBSRC
	infoxmlauth.cpp
)




add_library(
	auth
		STATIC
			${LIBSRC}
)



add_library( infodata::db::auth  ALIAS auth  )

target_include_directories(
    auth
	    PRIVATE
		../../include
		../../include/db
		../../include/db/auth
		${INCLUDE_PATHS}
		/usr/local/lib/eopenssl11
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(
    auth
	    PUBLIC
			${LIBRARIES}
)

	    
install(TARGETS auth DESTINATION lib/infokruncher/infodata/db/auth )

