add_subdirectory( db )


file(GLOB includes ${CMAKE_CURRENT_LIST_DIR}/*.h)

install(
    FILES
            ${includes}    
                    OPTIONAL
                    DESTINATION include/infokruncher
)

add_library(
    headers
            INTERFACE
)

add_library( kruncher::headers ALIAS headers )

target_include_directories(
    headers
            INTERFACE
                    $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
                    $<INSTALL_INTERFACE:include>
)

target_link_libraries(
    headers
            INTERFACE
)


