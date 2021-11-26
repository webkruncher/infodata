



#get_filename_component(INCLUDES_PATH ../include/db/ REALPATH) 

set (LIBSRC
	recordset.cpp
)




add_library(
	records
		STATIC
			${LIBSRC}
)



add_library( kruncher::db::records  ALIAS records  )

target_include_directories(
    records
	    PRIVATE
		../../include/
		../../include/db/
		../../include/db/auth
		../../include/db/records
		${INCLUDE_PATHS}
		/home/jmt/libdb/build_unix
		/usr/local/lib/eopenssl11
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(
    records
	    PUBLIC
			${LIBRARIES}
			kruncher::headers
			kruncher::db
			/home/jmt/libdb/build_unix/libdb_cxx.a
			ssl
			crypto
)

	    
install(TARGETS records DESTINATION lib/infokruncher/infodata/records )

