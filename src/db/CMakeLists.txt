



Headers(
	krunchercore
	exexml
	datakruncher
	informationkruncher
)


LibraryPaths(
	krunchercore::global/tracer/
	krunchercore::global/
	krunchercore::ut/Directory/
	krunchercore::ut/
	datafactory::data/
	informationkruncher::socks/
)


Libraries(
	krunchertrace
	krunchercore
	informationkruncher
)


add_subdirectory( auth )
add_subdirectory( records )
add_subdirectory( site )


add_executable(
	krestdb
		main.cpp
)


target_include_directories(
    krestdb
	    PRIVATE
		${INCLUDE_PATHS}
		../include/
		../include/db/
		../include/db/site/
		/home/jmt/libdb/build_unix
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include>
)


target_link_directories(
	krestdb
		PUBLIC
			${LIBRARY_PATHS}
)


target_link_libraries(
	krestdb
		PUBLIC
			ssl
			crypto
			site
			auth
			records
			${LIBRARIES}
)

install(
    TARGETS
	    krestdb
		    EXPORT exports
		    RUNTIME DESTINATION bin
) 


