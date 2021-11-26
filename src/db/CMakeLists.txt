
set(INCLUDE_PATHS)
IncludeProjects(
	krunchercore
	exexml
	datakruncher
	informationkruncher
)
set(LIBRARY_PATHS)
set(LIBRARIES)
IncludeLibraryPaths(
	krunchercore
	datakruncher
	informationkruncher
	infodata
	infodata/auth
	infodata/records
	infodata/site
)


add_subdirectory( auth )
add_subdirectory( records )
add_subdirectory( site )


add_executable(
	datafactory
		main.cpp
)


target_include_directories(
    datafactory
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
	datafactory
		PUBLIC
			${LIBRARY_PATHS}
)


target_link_libraries(
    datafactory
	    PUBLIC
			#infodata::db
			#infodata::headers
			#infodata::db::auth
			#infodata::db::site
			#infodata::db::records
			#/home/jmt/libdb/build_unix/libdb_cxx.a
			${LIBRARIES}
			informationkruncher
			ssl
			crypto
			krunchertrace
			site
			auth
)

install(
    TARGETS
	    datafactory
		    EXPORT exports
		    RUNTIME DESTINATION bin
) 


