
include_directories(
    ${CMAKE_SOURCE_DIR}
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
target_link_libraries( kcm_kdev_uisettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell sublime )

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
target_link_libraries( kcm_kdev_bgsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatformlanguage)

install( TARGETS kcm_kdev_bgsettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_bgsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

########### Background Parser

set( ccsettings_cfg_SRCS
    ccpreferences.cpp
)

set( ccsettings_cfg_UI
    ccsettings.ui
)

kde4_add_ui_files( ccsettings_cfg_SRCS ${ccsettings_cfg_UI} )
kde4_add_kcfg_files( ccsettings_cfg_SRCS ccconfig.kcfgc )
kde4_add_plugin( kcm_kdev_ccsettings ${ccsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_ccsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell ${KDE4_KTEXTEDITOR_LIBS})

install( TARGETS kcm_kdev_ccsettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_ccsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

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
target_link_libraries( kcm_kdev_projectsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell)

install( TARGETS kcm_kdev_projectsettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_projectsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

########### Run Options

set( runsettings_cfg_SRCS
    runpreferences/projectitemlineedit.cpp
    runpreferences/runpreferences.cpp
    runpreferences/runconfig.cpp
)

set( runsettings_cfg_UI
    runpreferences/runsettings.ui
    runpreferences/runconfig.ui
)

kde4_add_ui_files( runsettings_cfg_SRCS ${runsettings_cfg_UI} )
kde4_add_plugin( kcm_kdev_runsettings ${runsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_runsettings ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} kdevplatformshell kdevplatformutil kdevplatformproject kdevplatforminterfaces)

install( TARGETS kcm_kdev_runsettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_runsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )


########### Environment widget

set( envsettings_cfg_SRCS
  environmentwidget.cpp
  environmentgroupmodel.cpp
  environmentpreferences.cpp
)

set( envsettings_cfg_UI
  environmentwidget.ui
)

kde4_add_ui_files( envsettings_cfg_SRCS ${envsettings_cfg_UI} )
kde4_add_plugin( kcm_kdev_envsettings ${envsettings_cfg_SRCS} )
target_link_libraries( kcm_kdev_envsettings ${KDE4_KUTILS_LIBS} ${KDE4_KDEUI_LIBS} kdevplatformshell kdevplatformutil)

install( TARGETS kcm_kdev_envsettings DESTINATION ${PLUGIN_INSTALL_DIR} )
install( FILES kcm_kdev_envsettings.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

