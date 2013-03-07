
add_definitions( -DKDE_DEFAULT_DEBUG_AREA=9517 )

add_subdirectory(test)

########### next target ###############

set(kdevgenericmanager_PART_SRCS
    genericmanager.cpp
)

kde4_add_plugin(kdevgenericmanager ${kdevgenericmanager_PART_SRCS})
target_link_libraries(kdevgenericmanager ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} kdevplatforminterfaces kdevplatformproject kdevplatformlanguage )

install(TARGETS kdevgenericmanager DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

configure_file(kdevgenericmanager.desktop.cmake ${CMAKE_CURRENT_BINARY_DIR}/kdevgenericmanager.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/kdevgenericmanager.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


########### Generic Manager Options

set( genericprojectmanagersettings_cfg_SRCS
    genericprojectmanagerpreferences.cpp
)

set( genericprojectmanagersettings_cfg_UI
    genericprojectmanagersettings.ui
)

kde4_add_ui_files( genericprojectmanagersettings_cfg_SRCS ${genericprojectmanagersettings_cfg_UI} )
kde4_add_kcfg_files( genericprojectmanagersettings_cfg_SRCS genericprojectmanagersettings.kcfgc )
kde4_add_plugin( kcm_kdev_genericprojectmanagersettings ${genericprojectmanagersettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_genericprojectmanagersettings ${KDE4_KCMUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformproject kdevplatformutil kdevplatforminterfaces)

install( TARGETS kcm_kdev_genericprojectmanagersettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_genericprojectmanagersettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
