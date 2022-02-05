



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


#add_executable(
#	datatester
#		main.cpp
#)
#
#
#target_include_directories(
#    datatester
#	    PRIVATE
#		${INCLUDE_PATHS}
#		../include/
#		../include/db/site/
#		/home/jmt/libdb/build_unix
#		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
#		$<INSTALL_INTERFACE:include>
#)
#
#
#target_link_directories(
#	datatester
#		PUBLIC
#			${LIBRARY_PATHS}
#)
#
#
#target_link_libraries(
#	datatester
#		PUBLIC
#			ssl
#			crypto
#			site
#			auth
#			records
#			${LIBRARIES}
#)
#
#install(
#    TARGETS
#	    datatester
#		    EXPORT exports
#		    RUNTIME DESTINATION bin
#) 
#

