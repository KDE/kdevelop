set( compilerprovider_SRCS
        compilerprovider.cpp
        ../debugarea.cpp
    )

kde4_add_plugin( kdevcompilerprovider
        ${compilerprovider_SRCS})

target_link_libraries( kdevcompilerprovider LINK_PRIVATE
        ${KDEVPLATFORM_PROJECT_LIBRARIES}
        ${KDEVPLATFORM_UTIL_LIBRARIES}
        ${KDEVPLATFORM_LANGUAGE_LIBRARIES} )

install( TARGETS kdevcompilerprovider
         DESTINATION ${PLUGIN_INSTALL_DIR} )

configure_file(kdevcompilerprovider.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevcompilerprovider.desktop)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevcompilerprovider.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
