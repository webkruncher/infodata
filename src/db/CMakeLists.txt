


Banner()

execute_process( COMMAND bash -c "informationkrunchers | sed '/^#/d' | cut -d '|' -f2 | sort | uniq | sed 's/$/;/'  | tr -d '\n' " OUTPUT_VARIABLE KrunchDataServers )
foreach(KrunchDataServer IN LISTS KrunchDataServers )
	IF("${KrunchDataServer}" STREQUAL "" )
	ELSE()
		set(DataHeaders)
		set(DataLibPaths)
		set(DataLibs)
		set(RestInterface)
		set(MainEntryPoint)
		set(KruncherSpace)
		set(INCLUDE_PATHS)
		set(LIBRARY_PATHS)
		set(LIBRARIES)


		execute_process( COMMAND bash -c "informationkrunchers  | grep -e \"^|${KrunchDataServer}|\"  | cut -d '|' -f3 | sed 's/$/;/'  | tr -d '\n' " OUTPUT_VARIABLE DataHeaders )
		execute_process( COMMAND bash -c "informationkrunchers  | grep -e \"^|${KrunchDataServer}|\"  | cut -d '|' -f4 | sed 's/$/;/'  | tr -d '\n' " OUTPUT_VARIABLE DataLibPaths )
		execute_process( COMMAND bash -c "informationkrunchers  | grep -e \"^|${KrunchDataServer}|\"  | cut -d '|' -f5 | sed 's/$/;/'  | tr -d '\n' " OUTPUT_VARIABLE DataLibs )


		Headers(
			krunchercore
			krunchercore::tracer
			exexml
			datakruncher
			informationkruncher
			${DataHeaders}
		)


		LibraryPaths(
			krunchercore::global/tracer/
			krunchercore::global/
			krunchercore::ut/Directory/
			krunchercore::ut/
			datakruncher::data/
			informationkruncher::socks/
			${DataLibPaths}
		)


		Libraries(
			krunchertrace
			krunchercore
			informationkruncher
			datafactory
			${DataLibs}
			/home/jmt/libdb/build_unix/libdb_cxx.a
		)


		add_executable( ${KrunchDataServer}  
			infoservice.cpp
		)



		execute_process( COMMAND bash -c "informationkrunchers  | grep -e \"^|${KrunchDataServer}|\"  | cut -d '|' -f6 | sed 's/$/;/'  | tr -d '\n' " OUTPUT_VARIABLE RestInterface )
		execute_process( COMMAND bash -c "informationkrunchers  | grep -e \"^|${KrunchDataServer}|\"  | cut -d '|' -f7 | sed 's/$/;/'  | tr -d '\n' " OUTPUT_VARIABLE KruncherSpace )
		execute_process( COMMAND bash -c "informationkrunchers  | grep -e \"^|${KrunchDataServer}|\"  | cut -d '|' -f9 | sed 's/$/;/'  | tr -d '\n' " OUTPUT_VARIABLE MainEntryPoint )

		
		if(DEFINED ENV{KRUNCHER_DEV_VERBOSE})
			if ( ( $ENV{KRUNCHER_DEV_VERBOSE} EQUAL 2) OR ( $ENV{KRUNCHER_DEV_VERBOSE} EQUAL 3)  )
				message( "${YellowBkColor}${BlackColor}Target ${KrunchDataServer}${NormalColor}" )
				TraceListAlways( "DataHeaders " DataHeaders ${TealColor} )
				TraceListAlways( "DataLibPaths " DataLibPaths ${MagentaColor} )
				TraceListAlways( "DataLibs" DataLibs ${GreenColor} )
				message( "${MagentaBkColor}${BlackColor}RestInterface:${RestInterface}${NormalColor}" )
				message( "${YellowBkColor}${BlackColor}MainEntryPoint:${MainEntryPoint}${NormalColor}" )
			endif()
		endif()
		target_compile_options( 
			${KrunchDataServer} 
				PUBLIC 
					-DKruncherSpace=${KruncherSpace}
					-DRestInterface=${RestInterface}
					-DMainEntryPoint=${MainEntryPoint}
		)

		target_include_directories(
		    ${KrunchDataServer}
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
			${KrunchDataServer}
				PUBLIC
					${LIBRARY_PATHS}
		)


		target_link_libraries(
			${KrunchDataServer}
				PUBLIC
					ssl
					crypto
					${LIBRARIES}
		)

		install(
		    TARGETS
			    ${KrunchDataServer}
				    EXPORT exports
				    RUNTIME DESTINATION bin
		) 


	ENDIF()
endforeach()	


