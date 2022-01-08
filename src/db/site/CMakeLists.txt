




Headers(
	krunchercore
	krunchercore::tracer
	exexml
	datakruncher
	datakruncher::markets
	informationkruncher
)


LibraryPaths(
	krunchercore::global/tracer/
	krunchercore::global/
	krunchercore::ut/Directory/
	krunchercore::ut/
	datafactory::data/
	datakruncher::market/
	informationkruncher::socks/
)


Libraries(
	krunchertrace
	krunchercore
	marketdata
	informationkruncher
)




set (LIBSRC
	infodb.cpp
	infoservice.cpp
	infoerrors.cpp
)


add_library(
	site
		STATIC
			${LIBSRC}
)



add_library( kruncher::db::site  ALIAS site  )

target_include_directories(
    site
	    PRIVATE
		${INCLUDE_PATHS}
		/home/jmt/libdb/build_unix
		/usr/local/lib/eopenssl11
		../../include/
		../../include/db
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(
    site
	    PUBLIC
			${LIBRARIES}
			#kruncher::headers
			#kruncher::db
			/home/jmt/libdb/build_unix/libdb_cxx.a
			ssl
			crypto
)

	    
install(TARGETS site DESTINATION lib/infokruncher/infodata/db/site )

