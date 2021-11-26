
set(INCLUDE_PATHS)
IncludeProjects(
	krunchercore
	exexml
	informationkruncher
)
set(LIBRARY_PATHS)
set(LIBRARIES)
IncludeLibraryPaths(
	krunchercore
	informationkruncher
)

add_library(
	informationkruncher
		STATIC
			infokruncher.cpp
			infosite.cpp
			infoconsumer.cpp
			infoserver.cpp
			infofigur.cpp
)

add_library( kruncher::informationkruncher  ALIAS informationkruncher  )

target_include_directories(
    informationkruncher
	    PRIVATE
		${INCLUDE_PATHS}
		$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
		$<INSTALL_INTERFACE:include>
)

target_link_libraries(
    informationkruncher
	    PUBLIC
)

	    
install(TARGETS informationkruncher DESTINATION lib/infokruncher )

