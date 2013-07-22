
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9517 )

add_subdirectory(test)

########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

kde4_add_plugin(kdevgenericmanager ${kdevgenericmanager_PART_SRCS})
target_link_libraries(kdevgenericmanager
    ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS}
    kdevplatforminterfaces kdevplatformproject
)

install(TARGETS kdevgenericmanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevgenericmanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgenericmanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevgenericmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
