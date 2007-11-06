
include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/sublime
    ${CMAKE_SOURCE_DIR}/interfaces
    ${CMAKE_SOURCE_DIR}/editor
    ${CMAKE_SOURCE_DIR}/project
    ${CMAKE_SOURCE_DIR}/project/interfaces
    ${CMAKE_SOURCE_DIR}/language
    ${CMAKE_SOURCE_DIR}/language/backgroundparser
    ${CMAKE_SOURCE_DIR}/language/interfaces
    ${CMAKE_SOURCE_DIR}/shell
)

########### UI settings
set( uisettings_cfg_SRCS
    uipreferences.cpp
)

set( uisettings_cfg_UI
    uiconfig.ui
)

kde4_add_ui_files( uisettings_cfg_SRCS ${uisettings_cfg_UI} )
kde4_add_kcfg_files( uisettings_cfg_SRCS uiconfig.kcfgc )
kde4_add_plugin( kcm_kdev_uisettings ${uisettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_uisettings ${KDE4_KUTILS_LIBS} )

install( TARGETS kcm_kdev_uisettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_uisettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


########### Background Parser

set( bgsettings_cfg_SRCS
    bgpreferences.cpp
)

set( bgsettings_cfg_UI
    bgsettings.ui
)

kde4_add_ui_files( bgsettings_cfg_SRCS ${bgsettings_cfg_UI} )
kde4_add_kcfg_files( bgsettings_cfg_SRCS bgconfig.kcfgc )
kde4_add_plugin( kcm_kdev_bgsettings ${bgsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_bgsettings ${KDE4_KUTILS_LIBS} kdevplatformshell )

install( TARGETS kcm_kdev_bgsettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_bgsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

########### Project Configuration

set( projectsettings_cfg_SRCS
    projectpreferences.cpp
)

set( projectsettings_cfg_UI
    projectsettings.ui
)

kde4_add_ui_files( projectsettings_cfg_SRCS ${projectsettings_cfg_UI} )
kde4_add_kcfg_files( projectsettings_cfg_SRCS projectconfig.kcfgc )
kde4_add_plugin( kcm_kdev_projectsettings ${projectsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_projectsettings ${KDE4_KUTILS_LIBS} kdevplatformshell)

install( TARGETS kcm_kdev_projectsettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_projectsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
